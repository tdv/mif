//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include "mif/application/tcp_service_client.h"
#include "mif/common/log.h"
#include "mif/common/unused.h"
#include "mif/net/tcp/clients.h"
#include "mif/service/factory.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Application
    {

        namespace
        {

            class Factory
                : public Service::Inherit<Service::IFactory>
            {
            public:
                Factory(std::string const &host, std::string const &port, Net::IClientFactoryPtr clientFactory)
                    : m_host{host}
                    , m_port{port}
                    , m_clients{clientFactory}
                {
                }

            private:
                std::string m_host;
                std::string m_port;
                Net::Tcp::Clients m_clients;
                Net::IClientFactory::ClientPtr m_client;

                // IFactory
                virtual Service::IServicePtr Create(Service::ServiceId id) override final
                {
                    (void)id;
                    return {};
                }
            };

        }   // namespace

        TcpServiceClient::TcpServiceClient(int argc, char const **argv, ClientFactory const &clientFactory)
            : NetBaseApplication{argc, argv}
            , m_clientFactory{clientFactory}
        {
        }

        void TcpServiceClient::Init(Service::IFactoryPtr factory)
        {
            Common::Unused(factory);
        }

        void TcpServiceClient::Done()
        {
        }

        void TcpServiceClient::OnInit()
        {
            auto const host = GetHost();
            auto const port = GetPort();
            auto const workers = GetWorkers();
            auto const timeout = GetTimeout();

            MIF_LOG(Info) << "Starting client on " << host << ":" << port;


            auto factory = Service::Make<Service::Factory, Service::Factory>();
            auto clientFactory = m_clientFactory(workers, timeout, factory);

            m_factory = Service::Make<Factory, Service::IFactory>(host, port, clientFactory);

            Init(m_factory);

            MIF_LOG(Info) << "Client is successfully started.";
        }

        void TcpServiceClient::OnDone()
        {
            MIF_LOG(Info) << "Stopping client ...";

            m_factory.reset();

            try
            {
                Done();
            }
            catch (std::exception const &e)
            {
                MIF_LOG(Warning) << "[Mif::Application::TcpServiceClient::OnDone] "
                        << "Failed to call \"Done\". Error: " << e.what();
            }

            MIF_LOG(Info) << "Client is successfully stopped.";
        }

    }   // namespace Application
}   // namespace Mif
