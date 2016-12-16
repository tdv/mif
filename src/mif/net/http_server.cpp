//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>

// EVENT
#include <event2/util.h>

// MIF
#include "mif/common/log.h"
#include "mif/net/http_server.h"
#include "mif/net/http/server.h"

// THIS
#include "detail/http_constants.h"

namespace Mif
{
    namespace Net
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
                        return m_exception;
                    }

                    std::time_t GetTimestamp() const
                    {
                        return m_timestamp;
                    }

                    Common::Buffer OnData(Common::Buffer data)
                    {
                        try
                        {
                            if (!m_response.empty())
                                Common::Buffer{}.swap(m_response);
                            if (m_exception)
                                std::exception_ptr{}.swap(m_exception);

                            m_client->OnData(std::move(data));

                            m_timestamp = std::time(nullptr);
                        }
                        catch (std::exception const &e)
                        {
                            m_exception = std::current_exception();
                            CloseMe();
                            MIF_LOG(Warning) << "[Mif::Net::Detail::Session::OnData]. "
                                << "Failed to process data. Error: " << e.what();
                        }

                        return std::move(m_response);
                    }

                private:
                    std::time_t m_timestamp{std::time(nullptr)};
                    bool m_needForClose{false};
                    Common::Buffer m_response;
                    std::exception_ptr m_exception{};
                    IClientFactory::ClientPtr m_client;

                    //----------------------------------------------------------------------------
                    // IPublisher
                    virtual void Publish(Common::Buffer buffer) override
                    {
                        m_response.swap(buffer);
                    }

                    //----------------------------------------------------------------------------
                    // IControl
                    virtual void CloseMe() override
                    {
                        m_needForClose = true;
                    }

                    //----------------------------------------------------------------------------
                    void DoRead();
                };

            }   // namespace
        }   // namespace Detail

        class HTTPServer::Impl final
        {
        public:
            Impl(std::string const &host, std::string const &port,
                std::uint16_t workers, std::shared_ptr<IClientFactory> factory)
            try
                : m_factory{factory}
                , m_server{new Http::Server{host, port, workers, {Http::Method::Type::Post},
                    std::bind(&Impl::OnRequest, this, std::placeholders::_1, std::placeholders::_2)}}
            {
            }
            catch (std::exception const &e)
            {
                throw std::runtime_error{"[Mif::Net::HTTPServer::Impl] Failed to start server. "
                    "Error: " + std::string{e.what()}};
            }
            catch (...)
            {
                throw std::runtime_error{"[Mif::Net::HTTPServer::Impl] Failed to start server. Error: unknown."};
            }

            virtual ~Impl()
            {
                try
                {
                    m_server.reset();
                }
                catch (std::exception const &e)
                {
                    MIF_LOG(Error) << "[Mif::Net::HTTPServer::~Impl] Failed to stop server. Error: " << e.what();
                }
            }

        private:
            using LockType = std::mutex;
            using LockGuard = std::lock_guard<LockType>;

            using SessionPtr = std::shared_ptr<Detail::Session>;
            using Sessions = std::map<std::string/*session*/, SessionPtr>;

            LockType m_lock;

            Sessions m_sessions;
            std::shared_ptr<IClientFactory> m_factory;
            std::unique_ptr<Http::Server> m_server;

            void OnRequest(Http::IInputPack const &request, Http::IOutputPack &response)
            {
                std::string sessionId;

                try
                {
                    auto const headers = request.GetHeaders();
                    {
                        auto const iter = headers.find(Detail::HttpConstants::Header::Session::GetString());
                        if (iter != std::end(headers))
                            sessionId = iter->second;
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
                        throw std::invalid_argument{"No input data."};
                    else
                    {
                        auto responseData = session->OnData(std::move(requestData));
                        if (!session->NeedForClose())
                        {
                            response.SetData(std::move(responseData));
                            response.SetHeader(Detail::HttpConstants::Header::Session::GetString(), sessionId);
                            SetKeepAliveFromClient(headers, response);
                        }
                        else
                        {
                            auto ex = session->GetException();
                            if (ex)
                                std::rethrow_exception(ex);

                            response.SetHeader(
                                Detail::HttpConstants::Header::Connection::GetString(),
                                Detail::HttpConstants::Value::Connection::Close::GetString());
                        }
                    }
                }
                catch (std::invalid_argument const &e)
                {
                    MIF_LOG(Warning) << "[Mif::Net::HTTPServer::Impl::OnRequest] "
                        << "Failed to process request. Error: " << e.what();

                    RemoveSession(sessionId);
                    OnExceptionResponse(response, Http::Code::BadRequest, e.what());
                }
                catch (std::exception const &e)
                {
                    MIF_LOG(Warning) << "[Mif::Net::HTTPServer::Impl::OnRequest] "
                        << "Failed to process request. Error: " << e.what();

                    RemoveSession(sessionId);
                    OnExceptionResponse(response, Http::Code::Internal, e.what());
                }
                catch (...)
                {
                    MIF_LOG(Error) << "[Mif::Net::HTTPServer::Impl::OnRequest] "
                        << "Failed to process request. Error: unknown";

                    RemoveSession(sessionId);
                    OnExceptionResponse(response, Http::Code::Internal, "Unknown exception.");
                }
            }

            void SetKeepAliveFromClient(Http::IInputPack::Headers const &requestHeaders, Http::IOutputPack &response) const
            {
                auto const iter = requestHeaders.find(Detail::HttpConstants::Header::Connection::GetString());
                if (iter == std::end(requestHeaders) && !iter->second.empty())
                    return;
                if (!evutil_ascii_strcasecmp(iter->second.c_str(), "keep-alive"))
                {
                    response.SetHeader(Detail::HttpConstants::Header::Connection::GetString(),
                        Detail::HttpConstants::Value::Connection::KeepAlive::GetString());
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
                pack.SetHeader(Detail::HttpConstants::Header::Connection::GetString(),
                    Detail::HttpConstants::Value::Connection::Close::GetString());
                pack.SetCode(code);
                pack.SetData({std::begin(message), std::end(message)});
            }

        };

        HTTPServer::HTTPServer(std::string const &host, std::string const &port,
            std::uint16_t workers, std::shared_ptr<IClientFactory> factory)
            : m_impl{new Impl{host, port, workers, factory}}
        {
        }

        HTTPServer::~HTTPServer()
        {
        }

    }   // namespace Net
}   // namespace Mif
