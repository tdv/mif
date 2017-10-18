//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <ctime>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

// BOOST
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/scope_exit.hpp>

// EVENT
#include <event2/util.h>

// MIF
#include "mif/common/log.h"
#include "mif/net/http/constants.h"
#include "mif/net/http/servlet.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {
                namespace
                {

                    class Session final
                        : public std::enable_shared_from_this<Session>
                        , public IPublisher
                        , public IControl
                    {
                    public:
                        void Init(IClientFactory &factory)
                        {
                            auto self = shared_from_this();
                            m_client = factory.Create(std::weak_ptr<IControl>(self), std::weak_ptr<IPublisher>(self));
                        }

                        bool NeedForClose() const
                        {
                            return m_needForClose;
                        }

                        std::exception_ptr GetException() const
                        {
                            std::exception_ptr exception{};
                            {
                                LockGuard lock{m_lock};
                                auto iter = m_exceptions.find(std::this_thread::get_id());
                                if (iter != std::end(m_exceptions))
                                    return iter->second;
                            }
                            return {};
                        }

                        std::time_t GetTimestamp() const
                        {
                            return m_timestamp;
                        }

                        Common::Buffer OnData(Common::Buffer data)
                        {
                            auto const threadId = std::this_thread::get_id();

                            auto cleanResponse = [this, &threadId]
                                {
                                    LockGuard lock{m_lock};
                                    auto iter = m_responses.find(threadId);
                                    if (iter != std::end(m_responses))
                                        m_responses.erase(iter);
                                };

                            BOOST_SCOPE_EXIT(&cleanResponse)
                            {
                                cleanResponse();
                            }
                            BOOST_SCOPE_EXIT_END

                            try
                            {
                                cleanResponse();

                                {
                                    LockGuard lock{m_lock};
                                    auto iter = m_exceptions.find(std::this_thread::get_id());
                                    if (iter != std::end(m_exceptions))
                                        m_exceptions.erase(iter);
                                }

                                m_client->OnData(std::move(data));

                                m_timestamp = std::time(nullptr);

                                {
                                    LockGuard lock{m_lock};
                                    auto iter = m_responses.find(threadId);
                                    if (iter != std::end(m_responses))
                                        return std::move(iter->second);
                                }
                            }
                            catch (std::exception const &e)
                            {
                                {
                                    LockGuard lock{m_lock};
                                    m_exceptions.emplace(threadId, std::current_exception());
                                }
                                CloseMe();
                                MIF_LOG(Warning) << "[Mif::Net::Http::Detail::Session::OnData]. "
                                    << "Failed to process data. Error: " << e.what();
                            }

                            return {};
                        }

                    private:
                        using LockType = std::mutex;
                        using LockGuard = std::lock_guard<LockType>;

                        using Responses = std::map<std::thread::id, Common::Buffer>;
                        using Exceptions = std::map<std::thread::id, std::exception_ptr>;

                        mutable LockType m_lock;
                        std::time_t m_timestamp{std::time(nullptr)};
                        bool m_needForClose{false};
                        Responses m_responses;
                        Exceptions m_exceptions;
                        IClientFactory::ClientPtr m_client;

                        //----------------------------------------------------------------------------
                        // IPublisher
                        virtual void Publish(Common::Buffer buffer) override
                        {
                            LockGuard lock{m_lock};
                            m_responses.emplace(std::this_thread::get_id(), std::move(buffer));
                        }

                        //----------------------------------------------------------------------------
                        // IControl
                        virtual void CloseMe() override
                        {
                            m_needForClose = true;
                        }
                    };


                    class Servlet final
                    {
                    public:
                        Servlet(std::shared_ptr<IClientFactory> factory, std::uint32_t sessionTimeout)
                            : m_factory{factory}
                            , m_sessionTimeout{sessionTimeout}
                        {
                            m_iosWork.reset(new boost::asio::io_service::work{m_ioService});
                            m_iosWorker.reset(new std::thread{[this] { m_ioService.run(); }});
                            auto timer = std::make_shared<boost::asio::deadline_timer>(m_ioService,
                                    boost::posix_time::seconds{m_cleanerTimeout});
                            timer->async_wait(std::bind(&Servlet::OnCleaner, this, timer, std::placeholders::_1));
                        }

                        ~Servlet()
                        {
                            try
                            {
                                m_iosWork.reset();
                                m_ioService.stop();
                            }
                            catch (std::exception const &e)
                            {
                                MIF_LOG(Warning) << "[Mif::Net::Http::Detail::Servlet::~Servlet] "
                                        << "Error: " << e.what();
                            }
                        }

                        void OnRequest(IInputPack const &request, IOutputPack &response)
                        {
                            std::string sessionId;

                            try
                            {
                                auto const headers = request.GetHeaders();
                                {
                                    auto const iter = headers.find(Constants::Header::Session::Value);
                                    if (iter != std::end(headers))
                                        sessionId = iter->second;
                                    else
                                        throw std::invalid_argument{"Session not found."};
                                    if (sessionId.empty())
                                        throw std::invalid_argument{"Empty session."};
                                }

                                SessionPtr session;

                                {
                                    LockGuard lock{m_lock};
                                    auto iter = m_sessions.find(sessionId);
                                    if (iter != std::end(m_sessions))
                                        session = iter->second;
                                }

                                if (!session)
                                {
                                    session = std::make_shared<Detail::Session>();
                                    session->Init(*m_factory);

                                    LockGuard lock{m_lock};
                                    auto iter = m_sessions.find(sessionId);
                                    if (iter != std::end(m_sessions))
                                        session = iter->second;
                                    else if (!sessionId.empty())
                                        m_sessions.insert(std::make_pair(sessionId, session));
                                }

                                auto requestData = request.GetData();
                                if (requestData.empty())
                                {
                                    throw std::invalid_argument{"No input data."};
                                }
                                else
                                {
                                    auto responseData = session->OnData(std::move(requestData));
                                    response.SetData(std::move(responseData));
                                    if (!session->NeedForClose())
                                    {
                                        response.SetHeader(Constants::Header::Session::Value, sessionId);
                                        SetKeepAliveFromClient(headers, response);
                                    }
                                    else
                                    {
                                        auto ex = session->GetException();
                                        if (ex)
                                            std::rethrow_exception(ex);

                                        response.SetHeader(
                                            Constants::Header::Connection::Value,
                                            Constants::Value::Connection::Close::Value);
                                    }
                                }
                            }
                            catch (std::invalid_argument const &e)
                            {
                                MIF_LOG(Warning) << "[Mif::Net::Http::Detail::Servlet::OnRequest] "
                                    << "Failed to process request. Error: " << e.what();

                                RemoveSession(sessionId);
                                OnExceptionResponse(response, Code::BadRequest, e.what());
                            }
                            catch (std::exception const &e)
                            {
                                MIF_LOG(Warning) << "[Mif::Net::Http::Detail::Servlet::OnRequest] "
                                    << "Failed to process request. Error: " << e.what();

                                RemoveSession(sessionId);
                                OnExceptionResponse(response, Code::Internal, e.what());
                            }
                            catch (...)
                            {
                                MIF_LOG(Error) << "[Mif::Net::Http::Detail::Servlet::OnRequest] "
                                    << "Failed to process request. Error: unknown";

                                RemoveSession(sessionId);
                                OnExceptionResponse(response, Code::Internal, "Unknown exception.");
                            }
                        }

                    private:
                        using LockType = std::mutex;
                        using LockGuard = std::lock_guard<LockType>;

                        using SessionPtr = std::shared_ptr<Detail::Session>;
                        using Sessions = std::map<std::string, SessionPtr>;

                        std::shared_ptr<IClientFactory> m_factory;
                        std::uint32_t m_sessionTimeout;

                        std::int64_t const m_cleanerTimeout = 60;

                        boost::asio::io_service m_ioService;
                        std::unique_ptr<std::thread, void (*)(std::thread *)> m_iosWorker{nullptr, [] (std::thread *t) { if (t) t->join(); delete t; } };
                        std::unique_ptr<boost::asio::io_service::work> m_iosWork;

                        LockType m_lock;
                        Sessions m_sessions;

                        void SetKeepAliveFromClient(IInputPack::Headers const &requestHeaders, IOutputPack &response) const
                        {
                            auto const iter = requestHeaders.find(Constants::Header::Connection::Value);
                            if (iter == std::end(requestHeaders) && !iter->second.empty())
                                return;
                            if (!iter->second.empty() && !evutil_ascii_strcasecmp(iter->second.c_str(), "keep-alive"))
                            {
                                response.SetHeader(Constants::Header::Connection::Value,
                                        Constants::Value::Connection::KeepAlive::Value);
                            }
                        }

                        void RemoveSession(std::string const &sessionId)
                        {
                            if (sessionId.empty())
                                return;

                            SessionPtr session;

                            {
                                LockGuard lock{m_lock};
                                auto iter = m_sessions.find(sessionId);
                                if (iter != std::end(m_sessions))
                                {
                                    session = iter->second;
                                    m_sessions.erase(iter);
                                }
                            }
                        }

                        void OnExceptionResponse(Http::IOutputPack &pack, Http::Code code, std::string const &message)
                        {
                            pack.SetHeader(Constants::Header::Connection::Value,
                                Constants::Value::Connection::Close::Value);
                            pack.SetCode(code);
                            pack.SetData({std::begin(message), std::end(message)});
                        }

                        void OnCleaner(std::shared_ptr<boost::asio::deadline_timer>,
                                boost::system::error_code const &error)
                        {
                            if (error)
                            {
                                MIF_LOG(Warning) << "[Mif::Net::Http::Detail::Servlet::OnCleaner] "
                                        << "Failed to wati timer on clean. Error: " << error.message();
                            }

                            {
                                auto const now = std::time(nullptr);
                                LockGuard lock{m_lock};
                                for (auto i = std::begin(m_sessions) ; i != std::end(m_sessions) ; )
                                {
                                    if (i->second->NeedForClose() ||
                                            std::difftime(now, i->second->GetTimestamp()) >= m_sessionTimeout)
                                    {
                                        auto id = i->first;
                                        m_sessions.erase(i++);

                                        MIF_LOG(Info) << "[Mif::Net::Http::Detail::Servlet::OnCleaner] "
                                                << "Expired session with id \"" << id << "\" was deleted.";
                                    }
                                    else
                                    {
                                        ++i;
                                    }
                                }
                            }

                            auto timer = std::make_shared<boost::asio::deadline_timer>(m_ioService,
                                    boost::posix_time::seconds{m_cleanerTimeout});
                            timer->async_wait(std::bind(&Servlet::OnCleaner, this, timer, std::placeholders::_1));
                        }
                    };

                }   // namespace
            }   // namespace Detail

            ServerHandler MakeServlet(std::shared_ptr<IClientFactory> factory, std::uint32_t sessionTimeout)
            {
                auto adapter = std::make_shared<Detail::Servlet>(std::move(factory), sessionTimeout);
                auto handler = std::bind(&Detail::Servlet::OnRequest, adapter, std::placeholders::_1, std::placeholders::_2);
                return handler;
            }

        }   //namespace Http
    }   // namespace Net
}   // namespace Mif
