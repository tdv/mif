//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include "mif/application/http_server.h"
#include "mif/common/log.h"
#include "mif/common/unused.h"

namespace Mif
{
    namespace Application
    {

        HttpServer::HttpServer(int argc, char const **argv, Net::Http::Methods const &methods)
            : NetBaseApplication{argc, argv}
            , m_methods{methods}
        {
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
            //auto const timeout = GetTimeout();

            MIF_LOG(Info) << "Starting server on " << host << ":" << port;

            Net::Http::ServerHandlers handlers;

            Init(handlers);

            m_server.reset(new Net::Http::Server{host, port, workers, m_methods, handlers});

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
