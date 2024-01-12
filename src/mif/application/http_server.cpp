//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

// MIF
#include "mif/application/http_server.h"
#include "mif/common/log.h"
#include "mif/common/static_string.h"
#include "mif/common/unused.h"

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

                    using HttpBranch = MIF_STATIC_STR("http");
                    using HeadersLimit = MIF_STATIC_STR("headerslimit");
                    using BodyLimit = MIF_STATIC_STR("bodylimit");
                    using ChunkSize = MIF_STATIC_STR("chunksize");
                    using PipelineLimit = MIF_STATIC_STR("pipelinelimit");

                }   // namespace Config
            }   // namespace Detail
        }   // namespace

        HttpServer::HttpServer(int argc, char const **argv, Net::Http::Methods const &methods)
            : NetBaseApplication{argc, argv}
            , m_methods{methods}
        {
            boost::program_options::options_description options{"HttpServer options"};

            options.add_options()
                    (Detail::Config::HeadersLimit::Value, boost::program_options::value<std::size_t >(&m_headersLimit)->default_value(1024 * 1024), "Max length of headers (bytes)")
                    (Detail::Config::BodyLimit::Value, boost::program_options::value<std::size_t >(&m_bodyLimit)->default_value(10 * 1024 * 1024), "Max body length (bytes)")
                    (Detail::Config::ChunkSize::Value, boost::program_options::value<std::size_t >(&m_chunkSize)->default_value(10 * 1024 * 1024), "Data chunk size (bytes)")
                    (Detail::Config::PipelineLimit::Value, boost::program_options::value<std::size_t >(&m_pipelineLimit)->default_value(8), "Max items count in pipeline");

            AddCustomOptions(options);
        }

        void HttpServer::Init(Net::Http::ServerHandlers &handlers)
        {
            Common::Unused(handlers);
        }

        void HttpServer::Done()
        {
        }

        void HttpServer::OnInit()
        {
            auto const host = GetHost();
            auto const port = GetPort();
            auto const workers = GetWorkers();

            auto const timeout = std::max<std::size_t>(GetTimeout().count(), 100);

            if (auto config = GetConfig())
            {
                auto const branch = (GetConfigBranch() + ".") + Detail::Config::HttpBranch::Value;
                if (auto limits = config->GetConfig(branch))
                {
                    if (limits->Exists(Detail::Config::HeadersLimit::Value))
                        m_headersLimit = limits->GetValue<std::size_t>(Detail::Config::HeadersLimit::Value);

                    if (limits->Exists(Detail::Config::BodyLimit::Value))
                        m_bodyLimit = limits->GetValue<std::size_t>(Detail::Config::BodyLimit::Value);

                    if (limits->Exists(Detail::Config::ChunkSize::Value))
                        m_chunkSize = limits->GetValue<std::size_t>(Detail::Config::ChunkSize::Value);

                    if (limits->Exists(Detail::Config::PipelineLimit::Value))
                        m_pipelineLimit = limits->GetValue<std::size_t>(Detail::Config::PipelineLimit::Value);
                }
            }

            auto const headersLimit = std::max<std::size_t>(m_headersLimit, 128);
            auto const bodyLimit = std::max<std::size_t>(m_bodyLimit, 256);
            auto const chunkSize = std::max<std::size_t>(m_chunkSize, 256);
            auto const pipelineLimit = std::max<std::size_t>(m_pipelineLimit, 1);

            MIF_LOG(Info) << "Starting server with host \"" << host << "\", "
                          << "port \"" << port << "\", "
                          << "timeout \"" << timeout << "\", "
                          << "workers \"" << workers << "\", "
                             << "headers limit \"" << headersLimit << "\", "
                             << "body limit \"" << bodyLimit << "\", "
                                << "chunk size \"" << chunkSize << "\", "
                             << "pipeline limit \"" << pipelineLimit << "\" ";

            Net::Http::ServerHandlers handlers;

            Init(handlers);

            m_server.reset(new Net::Http::Server{host, port, workers, m_methods,
                handlers, headersLimit, bodyLimit, chunkSize, timeout, pipelineLimit});

            MIF_LOG(Info) << "Server is successfully started.";
        }

        void HttpServer::OnDone()
        {
            MIF_LOG(Info) << "Stopping server ...";

            m_server.reset();

            try
            {
                Done();
            }
            catch (std::exception const &e)
            {
                MIF_LOG(Warning) << "[Mif::Application::HttpServer::OnDone] "
                        << "Failed to call \"Done\". Error: " << e.what();
            }

            MIF_LOG(Info) << "Server is successfully stopped.";
        }

    }   // namespace Application
}   // namespace Mif
