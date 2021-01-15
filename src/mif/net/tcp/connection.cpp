//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

// STD
#include <utility>
#include <memory>
#include <mutex>

// MIF
#include "mif/net/tcp/clients.h"
#include "mif/net/tcp/connection.h"

namespace Mif
{
    namespace Net
    {
        namespace Tcp
        {

            class Connection::Impl final
            {
            public:
                Impl(std::string const &host, std::string const &port,
                        IClientFactoryPtr factory)
                    : m_host{host}
                    , m_port{port}
                    , m_clients{std::move(factory)}
                {
                }

                IConnection::ClientPtr GetClient()
                {
                    IConnection::ClientPtr client;

                    {
                        LockGuard lock{m_lock};
                        if (!m_client || m_client->IsClosed())
                            m_client = m_clients.RunClient(m_host, m_port);
                        client = m_client;
                    }

                    return client;
                }

            private:
                using LockType = std::mutex;
                using LockGuard = std::lock_guard<LockType>;

                LockType m_lock;

                std::string m_host;
                std::string m_port;
                Clients m_clients;
                IConnection::ClientPtr m_client;
            };


            Connection::Connection(std::string const &host, std::string const &port,
                        IClientFactoryPtr factory)
                : m_impl{new Impl{host, port, std::move(factory)}}
            {
            }

            Connection::~Connection()
            {
            }

            IConnection::ClientPtr Connection::GetClient()
            {
                return m_impl->GetClient();
            }

        }   // namespace Tcp
    }   // namespace Net
}   // namespace Mif
