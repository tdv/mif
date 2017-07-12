//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include "mif/application/http_server.h"
#include "mif/common/log.h"
#include "mif/common/static_string.h"
#include "mif/common/unused.h"
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

            }   // namespace Config
        }   // namespace Detail

        HttpServer::HttpServer(int argc, char const **argv, Net::Http::Methods const &methods)
            : Application{argc, argv}
            , m_methods{methods}
        {
            boost::program_options::options_description options{"Server options"};
            options.add_options()
                    (Detail::Config::ServerHost::GetString(), boost::program_options::value<std::string>()->default_value("0.0.0.0"), "Server host")
                    (Detail::Config::ServerPort::GetString(), boost::program_options::value<std::string>()->default_value("55555"), "Server port")
                    (Detail::Config::ServerWprkers::GetString(), boost::program_options::value<std::uint16_t>()->default_value(8), "Workers thread count");

            AddCustomOptions(options);
        }

        void HttpServer::Init(Net::Http::ServerHandlers &handlers)
        {
            Common::Unused(handlers);
        }

        void HttpServer::Done()
        {
        }

        void HttpServer::OnStart()
        {
            std::string host;
            std::string port;
            std::uint16_t workers = 0;

            if (auto config = GetConfig())
            {
                if (config->Exists(Detail::Config::ServerBranch::GetString()))
                {
                    auto serverConfig = config->GetConfig(Detail::Config::ServerBranch::GetString());
                    host = serverConfig->GetValue(Detail::Config::ServerHost::GetString());
                    port = serverConfig->GetValue(Detail::Config::ServerPort::GetString());
                    workers = serverConfig->GetValue<std::uint16_t>(Detail::Config::ServerWprkers::GetString());
                }
                else
                {
                    MIF_LOG(Warning) << "[Mif::Application::HttpServer::OnStart] "
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

            MIF_LOG(Info) << "Starting server on " << host << ":" << port;

            Net::Http::ServerHandlers handlers;
            Init(handlers);

            m_server.reset(new Net::Http::Server{host, port, workers, m_methods, handlers});

            MIF_LOG(Info) << "Server is successfully started.";
        }

        void HttpServer::OnStop()
        {
            MIF_LOG(Info) << "Stopping server ...";

            m_server.reset();

            try
            {
                Done();
            }
            catch (std::exception const &e)
            {
                MIF_LOG(Warning) << "[Mif::Application::HttpServer::OnStop] "
                        << "Failed to call \"Done\". Error: " << e.what();
            }

            Service::RootLocator::Get()->Clear();

            MIF_LOG(Info) << "Server is successfully stopped.";
        }

    }   // namespace Application
}   // namespace Mif
