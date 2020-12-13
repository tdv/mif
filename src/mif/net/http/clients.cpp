//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>
#include <map>
#include <memory>
#include <mutex>
#include <utility>

// MIF
#include "mif/common/log.h"
#include "mif/common/uuid_generator.h"
#include "mif/net/http/clients.h"
#include "mif/net/http/connection.h"
#include "mif/net/http/constants.h"

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
                        using OnCloseHandler = std::function<void (std::string const &)>;

                        Session(std::string const &host, std::string const &port, std::string const &resource,
                                OnCloseHandler const &onCloseHandler)
                            : m_host{host}
                            , m_port{port}
                            , m_resource{resource}
                            , m_sessionId{Common::UuidGenerator{}.Generate()}
                            , m_onCloseHandler{onCloseHandler}
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

                        using ConnectionPtr = std::shared_ptr<Connection>;

                        LockType m_lock;

                        std::string m_host;
                        std::string m_port;
                        std::string m_resource;
                        std::string m_sessionId;
                        OnCloseHandler m_onCloseHandler;

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
                                    auto self = shared_from_this();
                                    m_connection = std::make_shared<Connection>(m_host, m_port,
                                        std::bind(&Session::OnRequestDone, self, std::placeholders::_1),
                                        std::bind(&Session::OnClose, self));
                                }
                                connection = m_connection;
                            }
                            return connection;
                        }

                        void OnRequestDone(IInputPack const &pack)
                        {
                            try
                            {
                                {
                                    auto const headers = pack.GetHeaders();
                                    auto const sessionIter = headers.find(Constants::Header::MifExt::Session::Value);
                                    if (sessionIter == std::end(headers))
                                        throw std::runtime_error{"No session from server."};
                                    if (sessionIter->second.empty())
                                        throw std::runtime_error{"Empty session from server."};
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

                                MIF_LOG(Error) << "[Mif::Net::Http::Clients::Impl::OnRequestDone] Failed tp process data. "
                                    << "Error: " << e.what();
                            }
                            catch (...)
                            {
                                CloseMe();

                                MIF_LOG(Error) << "[Mif::Net::Http::Clients::Impl::OnRequestDone] Failed tp process data. "
                                    << "Error: unknown";
                            }
                        }

                        void OnClose()
                        {
                            // TODO: may be try to reconnect
                        }

                        //----------------------------------------------------------------------------
                        // IPublisher
                        virtual void Publish(Common::Buffer buffer) override final
                        {
                            try
                            {
                                auto connection = GetConnection();
                                auto pack = connection->CreateRequest();

                                pack->SetHeader(Constants::Header::MifExt::Session::Value, m_sessionId);

                                pack->SetData(std::move(buffer));

                                connection->MakeRequest(Method::Type::Post, m_resource, std::move(pack));
                            }
                            catch (std::exception const &e)
                            {
                                MIF_LOG(Error) << "[Mif::Net::Http::Clients::Impl::Publish] Failed tp Publish data. "
                                    << "Error: " << e.what();

                                CloseMe();
                            }
                            catch (...)
                            {
                                MIF_LOG(Error) << "[Mif::Net::Http::Clients::Impl::Publish] Failed tp Publish data. "
                                    << "Error: unknown";

                                CloseMe();
                            }
                        }

                        //----------------------------------------------------------------------------
                        // IControl
                        virtual void CloseMe() override final
                        {
                            m_onCloseHandler(m_sessionId);

                            try
                            {
                                m_client->OnClose();
                            }
                            catch (std::exception const &e)
                            {
                                MIF_LOG(Error) << "[Mif::Net::Http::Clients::Impl::CloseMe] Failed to call OnClose for client. "
                                    << "Error: " << e.what();
                            }
                            catch (...)
                            {
                                MIF_LOG(Error) << "[Mif::Net::Http::Clients::Impl::CloseMe] Failed to call OnClose for client. "
                                    << "Error: unknown";
                            }

                            m_needForClose = true;

                            OnClose();
                        }

                    };

                }   // namespace
            }   // namespace Detail

            class Clients::Impl final
            {
            public:
                Impl(std::shared_ptr<IClientFactory> factory)
                    : m_factory{std::move(factory)}
                {
                }

                ~Impl()
                {
                }

                IClientFactory::ClientPtr RunClient(std::string const &host, std::string const &port, std::string const &resource)
                {
                    try
                    {
                        auto lock = m_lock;
                        auto sessions = m_sessions;
                        auto session = std::make_shared<Detail::Session>(host, port, resource,
                                [lock, sessions] (std::string const &id)
                                {
                                    SessionPtr session;
                                    {
                                        LockGuard guard{*lock};
                                        auto iter = sessions->find(id);
                                        if (iter == std::end(*sessions))
                                        {
                                            MIF_LOG(Warning) << "[Mif::Net::Http::Clients::Impl::RunClient] "
                                                << "Session \"" + id + "\" not found.";
                                        }
                                        else
                                        {
                                            std::swap(session, iter->second);
                                            sessions->erase(iter);
                                        }
                                    }
                                }
                            );
                        auto client = session->Init(*m_factory);
                        {
                            LockGuard lock{*m_lock};
                            m_sessions->emplace(session->GetId(), session);
                        }
                        return client;
                    }
                    catch (std::exception const &e)
                    {
                        throw std::runtime_error{"[Mif::Net::Http::Clients::Impl::RunClient] Failed tp run client "
                            "on host \"" + host + "\" and port \"" + port + "\". "
                            "Error: " + std::string{e.what()}};
                    }
                }

            private:
                std::shared_ptr<IClientFactory> m_factory;
                using SessionPtr = std::shared_ptr<Detail::Session>;
                using Sessions = std::map<std::string, SessionPtr>;
                using SessionsPtr = std::shared_ptr<Sessions>;

                using LockType = std::mutex;
                using LockTypePtr = std::shared_ptr<LockType>;
                using LockGuard = std::lock_guard<LockType>;

                LockTypePtr m_lock = std::make_shared<LockType>();
                SessionsPtr m_sessions = std::make_shared<Sessions>();
            };


            Clients::Clients(std::shared_ptr<IClientFactory> factory)
                : m_impl{new Clients::Impl{std::move(factory)}}
            {
            }

            Clients::~Clients()
            {
            }

            IClientFactory::ClientPtr Clients::RunClient(std::string const &host, std::string const &port,
                    std::string const &resource)
            {
                return m_impl->RunClient(host, port, resource);
            }

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
