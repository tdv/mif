//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

// STD
#include <mutex>

// MIF
#include "mif/application/tcp_service_client.h"
#include "mif/common/log.h"
#include "mif/common/unused.h"
#include "mif/net/tcp/connection.h"
#include "mif/remote/factory.h"
#include "mif/service/factory.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Application
    {

        TcpServiceClient::TcpServiceClient(int argc, char const **argv,
                    ClientFactory const &clientFactory, ServiceCreator const &serviceCreator)
            : NetBaseApplication{argc, argv}
            , m_clientFactory{clientFactory}
            , m_serviceCreator{serviceCreator}
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

            auto connection = std::make_shared<Net::Tcp::Connection>(host, port, clientFactory);
            auto remoteFactory = Service::Make<Remote::Factory, Service::IFactory>(connection, m_serviceCreator);

            Init(remoteFactory);

            MIF_LOG(Info) << "Client is successfully started.";
        }

        void TcpServiceClient::OnDone()
        {
            MIF_LOG(Info) << "Stopping client ...";

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
