//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>
#include <map>
#include <mutex>
#include <utility>

// MIF
#include "mif/common/uuid_generator.h"
#include "mif/net/http_clients.h"

// THIS
#include "mif/common/log.h"
#include "mif/net/http/connection.h"

// TODO: delete this
#include <thread>
#include <chrono>

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
                    Session(std::string const &host, std::string const &port)
                        : m_host{host}
                        , m_port{port}
                        , m_sessionId{Common::UuidGenerator{}.Generate()}
                    {
                    }

                    std::string GetId() const
                    {
                        return m_sessionId;
                    }

                    IClientFactory::ClientPtr Init(IClientFactory &factory)
                    {
                        GetConnection();
                        m_client = factory.Create(std::weak_ptr<IControl>(shared_from_this()),
                            std::weak_ptr<IPublisher>(shared_from_this()));
                        return m_client;
                    }

                private:
                    using LockType = std::mutex;
                    using LockGuard = std::lock_guard<LockType>;

                    using ConnectionPtr = std::shared_ptr<Http::Connection>;

                    LockType m_lock;

                    std::string m_host;
                    std::string m_port;
                    std::string m_sessionId;

                    bool m_needForClose{false};

                    ConnectionPtr m_connection;

                    IClientFactory::ClientPtr m_client;

                    ConnectionPtr GetConnection()
                    {
                        ConnectionPtr connection;
                        {
                            LockGuard lock{m_lock};
                            
                            if (m_needForClose)
                                throw std::runtime_error{"Session marked for closure."};

                            if (!m_connection || m_connection->IsClosed())
                            {
                                m_connection = std::make_shared<Http::Connection>(m_host, m_port,
                                    std::bind(&Session::OnRequestDone, shared_from_this(), std::placeholders::_1));
                            }
                            connection = m_connection;
                        }
                        return connection;
                    }

                    void OnRequestDone(Http::IInputPack const &pack)
                    {
                        try
                        {
                            {
                                auto const headers = pack.GetHeaders();
                                auto const sessionIter = headers.find("X-Mif-Session");
                                if (sessionIter == std::end(headers))
                                    throw std::runtime_error{"No session from server."};
                                if (sessionIter->second != m_sessionId)
                                {
                                    throw std::runtime_error{"Bad session from server. "
                                        "Server session: \"" + sessionIter->second + "\" "
                                        "Needed session: \"" + m_sessionId + "\""};
                                }
                            }

                            auto data = pack.GetData();
                            if (data.empty())
                                throw std::runtime_error{"No data in the server response."};
                            m_client->OnData(std::move(data));
                        }
                        catch (std::exception const &e)
                        {
                            CloseMe();
                            
                            MIF_LOG(Error) << "[Mif::Net::HTTPClients::Impl::OnRequestDone] Failed tp process data. "
                                << "Error: " << e.what();
                        }
                        catch (...)
                        {
                            CloseMe();

                            MIF_LOG(Error) << "[Mif::Net::HTTPClients::Impl::OnRequestDone] Failed tp process data. "
                                << "Error: unknown";
                        }
                    }

                    //----------------------------------------------------------------------------
                    // IPublisher
                    virtual void Publish(Common::Buffer buffer) override final
                    {
                        try
                        {
                            auto connection = GetConnection();
                            auto pack = connection->CreateRequest();
                            
                            pack->SetHeader("Connection", "keep-alive");
                            pack->SetHeader("X-Mif-Session", m_sessionId);

                            pack->SetData(std::move(buffer));

                            connection->MakeRequest(Http::Method::Type::Post, "", std::move(pack));
                        }
                        catch (std::exception const &e)
                        {
                            CloseMe();
                            
                            MIF_LOG(Error) << "[Mif::Net::HTTPClients::Impl::Publish] Failed tp Publish data. "
                                << "Error: " << e.what();
                        }
                        catch (...)
                        {
                            CloseMe();

                            MIF_LOG(Error) << "[Mif::Net::HTTPClients::Impl::Publish] Failed tp Publish data. "
                                << "Error: unknown";
                        }
                    }

                    //----------------------------------------------------------------------------
                    // IControl
                    virtual void CloseMe() override final
                    {
                        LockGuard lock{m_lock};
                        m_connection.reset();
                        m_needForClose = true;
                    }

                };

            }   // namespace
        }   // namespace Detail

        class HTTPClients::Impl final
        {
        public:
            Impl(std::shared_ptr<IClientFactory> factory)
                : m_factory(factory)
            {
            }

            ~Impl()
            {
                try
                {
                    // TODO:
                }
                catch (std::exception const &e)
                {
                    MIF_LOG(Error) << "[Mif::Net::HTTPClients::Impl] Failed tp stop clients. "
                        << "Error: " << e.what();
                }
                catch (...)
                {
                    MIF_LOG(Error) << "[Mif::Net::HTTPClients::Impl] Failed tp stop clients. Error: unknown.";
                }
            }

            IClientFactory::ClientPtr RunClient(std::string const &host, std::string const &port)
            {
                try
                {
                    auto session = std::make_shared<Detail::Session>(host, port);
                    auto client = session->Init(*m_factory);
                    m_sessions[session->GetId()] = session;
                    return client;
                }
                catch (std::exception const &e)
                {
                    throw std::runtime_error{"[Mif::Net::HTTPClients::Impl::RunClient] Failed tp run client "
                        "on host \"" + host + "\" and port \"" + port + "\". "
                        "Error: " + std::string{e.what()}};
                }
            }

        private:
            std::shared_ptr<IClientFactory> m_factory;
            using SessionPtr = std::shared_ptr<Detail::Session>;
            using Sessions = std::map<std::string, SessionPtr>;
            Sessions m_sessions;
        };


        HTTPClients::HTTPClients(std::shared_ptr<IClientFactory> factory)
            : m_impl{new HTTPClients::Impl{factory}}
        {
        }

        HTTPClients::~HTTPClients()
        {
        }

        IClientFactory::ClientPtr HTTPClients::RunClient(std::string const &host, std::string const &port)
        {
            return m_impl->RunClient(host, port);
        }


    }   // namespace Net
}   // namespace Mif
