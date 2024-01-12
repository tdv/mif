//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

// MIF
#include "mif/application/tcp_service.h"
#include "mif/common/log.h"
#include "mif/common/unused.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Application
    {

        TcpService::TcpService(int argc, char const **argv, ClientFactory const &clientFactory)
            : NetBaseApplication{argc, argv}
            , m_clientFactory{clientFactory}
        {
        }

        void TcpService::Init(Service::FactoryPtr factory)
        {
            Common::Unused(factory);
        }

        void TcpService::Done()
        {
        }

        void TcpService::OnInit()
        {
            auto const host = GetHost();
            auto const port = GetPort();
            auto const workers = GetWorkers();
            auto const timeout = GetTimeout();

            MIF_LOG(Info) << "Starting server on " << host << ":" << port;

            auto factory = Service::Make<Service::Factory, Service::Factory>();

            Init(factory);

            auto clientFactory = m_clientFactory(workers, timeout, factory);

            m_server.reset(new Net::Tcp::Server{host, port, clientFactory});

            MIF_LOG(Info) << "Server is successfully started.";
        }

        void TcpService::OnDone()
        {
            MIF_LOG(Info) << "Stopping server ...";

            m_server.reset();

            try
            {
                Done();
            }
            catch (std::exception const &e)
            {
                MIF_LOG(Warning) << "[Mif::Application::TcpService::OnDone] "
                        << "Failed to call \"Done\". Error: " << e.what();
            }

            MIF_LOG(Info) << "Server is successfully stopped.";
        }

    }   // namespace Application
}   // namespace Mif
