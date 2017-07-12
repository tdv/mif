//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <chrono>

// MIF
#include "mif/application/tcp_service.h"
#include "mif/common/log.h"
#include "mif/common/static_string.h"
#include "mif/common/unused.h"
#include "mif/service/make.h"
#include "mif/service/root_locator.h"

namespace Mif
{
    namespace Application
    {

        namespace Detail
        {
            namespace Config
            {

                MIF_DECLARE_SRTING_PROVIDER(ServerBranch, "server")
                MIF_DECLARE_SRTING_PROVIDER(ServerHost, "host")
                MIF_DECLARE_SRTING_PROVIDER(ServerPort, "port")
                MIF_DECLARE_SRTING_PROVIDER(ServerWprkers, "workers")
                MIF_DECLARE_SRTING_PROVIDER(ServerTimeout, "timeout")

            }   // namespace Config
        }   // namespace Detail

        TcpService::TcpService(int argc, char const **argv, ClientFactory const &clientFactory)
            : Application{argc, argv}
            , m_clientFactory{clientFactory}
        {
            boost::program_options::options_description options{"Server options"};
            options.add_options()
                    (Detail::Config::ServerHost::GetString(), boost::program_options::value<std::string>()->default_value("0.0.0.0"), "Server host")
                    (Detail::Config::ServerPort::GetString(), boost::program_options::value<std::string>()->default_value("55555"), "Server port")
                    (Detail::Config::ServerWprkers::GetString(), boost::program_options::value<std::uint16_t>()->default_value(8), "Workers thread count")
                    (Detail::Config::ServerTimeout::GetString(), boost::program_options::value<std::uint64_t>()->default_value(10 * 1000 * 1000), "Time of request processing (microseconds)");

            AddCustomOptions(options);
        }

        void TcpService::Init(Service::FactoryPtr factory)
        {
            Mif::Common::Unused(factory);
        }

        void TcpService::Done()
        {
        }

        void TcpService::OnStart()
        {
            std::string host;
            std::string port;
            std::uint16_t workers = 0;
            std::chrono::microseconds timeout{0};

            if (auto config = GetConfig())
            {
                if (config->Exists(Detail::Config::ServerBranch::GetString()))
                {
                    auto serverConfig = config->GetConfig(Detail::Config::ServerBranch::GetString());
                    host = serverConfig->GetValue(Detail::Config::ServerHost::GetString());
                    port = serverConfig->GetValue(Detail::Config::ServerPort::GetString());
                    workers = serverConfig->GetValue<std::uint16_t>(Detail::Config::ServerWprkers::GetString());
                    timeout = std::chrono::microseconds{
                            serverConfig->GetValue<std::uint64_t>(Detail::Config::ServerTimeout::GetString())
                        };
                }
                else
                {
                    MIF_LOG(Warning) << "[Mif::Application::TcpService::OnStart] "
                            << "Branch \"" << Detail::Config::ServerBranch::GetString() << "\" not found. "
                            << "All server parameters will be taken from the command line arguments or default parameters will be used.";
                }
            }

            auto const &options = GetOptions();

            if (host.empty())
                host = options[Detail::Config::ServerHost::GetString()].as<std::string>();
            if (port.empty())
                port = options[Detail::Config::ServerPort::GetString()].as<std::string>();
            if (!workers)
                workers = options[Detail::Config::ServerWprkers::GetString()].as<std::uint16_t>();
            if (!timeout.count())
                timeout = std::chrono::microseconds{options[Detail::Config::ServerTimeout::GetString()].as<std::uint64_t>()};

            MIF_LOG(Info) << "Starting server on " << host << ":" << port;

            auto factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::Factory>();

            Init(factory);

            auto clientFactory = m_clientFactory(workers, timeout, factory);

            m_server.reset(new Mif::Net::Tcp::Server{host, port, clientFactory});

            MIF_LOG(Info) << "Server is successfully started.";
        }

        void TcpService::OnStop()
        {
            MIF_LOG(Info) << "Stopping server ...";

            m_server.reset();

            Service::RootLocator::Get()->Clear();

            MIF_LOG(Info) << "Server is successfully stopped.";
        }

    }   // namespace Application
}   // namespace Mif
