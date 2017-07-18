//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include "mif/application/net_base_application.h"
#include "mif/common/log.h"
#include "mif/common/static_string.h"

namespace Mif
{
    namespace Application
    {

        namespace
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
        }   // namespace

        NetBaseApplication::NetBaseApplication(int argc, char const **argv)
            : Application{argc, argv}
        {
            boost::program_options::options_description options{"Network options"};
            options.add_options()
                    (Detail::Config::ServerHost::GetString(), boost::program_options::value<std::string>(&m_host)->default_value("0.0.0.0"), "Server host")
                    (Detail::Config::ServerPort::GetString(), boost::program_options::value<std::string>(&m_port)->default_value("55555"), "Server port")
                    (Detail::Config::ServerWprkers::GetString(), boost::program_options::value<std::uint16_t>(&m_workers)->default_value(8), "Workers thread count")
                    (Detail::Config::ServerTimeout::GetString(), boost::program_options::value<std::uint64_t>(&m_timeout)->default_value(10 * 1000 * 1000), "Time of request processing (microseconds)");

            AddCustomOptions(options);
        }

        void NetBaseApplication::OnInit()
        {
        }

        void NetBaseApplication::OnDone()
        {
        }

        std::string NetBaseApplication::GetHost() const
        {
            return m_host;
        }

        std::string NetBaseApplication::GetPort() const
        {
            return m_port;
        }

        std::uint16_t NetBaseApplication::GetWorkers() const
        {
            return m_workers;
        }

        std::chrono::microseconds NetBaseApplication::GetTimeout() const
        {
            return std::chrono::microseconds{m_timeout};
        }

        void NetBaseApplication::OnStart()
        {
            if (auto config = GetConfig())
            {
                if (config->Exists(Detail::Config::ServerBranch::GetString()))
                {
                    auto serverConfig = config->GetConfig(Detail::Config::ServerBranch::GetString());
                    m_host = serverConfig->GetValue(Detail::Config::ServerHost::GetString());
                    m_port = serverConfig->GetValue(Detail::Config::ServerPort::GetString());
                    m_workers = serverConfig->GetValue<std::uint16_t>(Detail::Config::ServerWprkers::GetString());
                    m_timeout = serverConfig->GetValue<std::uint64_t>(Detail::Config::ServerTimeout::GetString());
                }
                else
                {
                    MIF_LOG(Warning) << "[Mif::Application::NetBaseApplication::OnStart] "
                            << "Config branch \"" << Detail::Config::ServerBranch::GetString() << "\" not found. "
                            << "All network parameters will be taken from the command line arguments or default parameters will be used.";
                }
            }

            MIF_LOG(Info) << "Starting network application on " << m_host << ":" << m_port;

            OnInit();

            MIF_LOG(Info) << "Network application is successfully started.";
        }

        void NetBaseApplication::OnStop()
        {
            MIF_LOG(Info) << "Stopping network application ...";

            try
            {
                OnDone();
            }
            catch (std::exception const &e)
            {
                MIF_LOG(Warning) << "[Mif::Application::NetBaseApplication::OnStop] "
                        << "Failed to call \"Done\". Error: " << e.what();
            }

            MIF_LOG(Info) << "Network application is successfully stopped.";
        }

    }   // namespace Application
}   // namespace Mif
