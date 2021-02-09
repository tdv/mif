//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_APPLICATION_HTTP_SERVER_H__
#define __MIF_APPLICATION_HTTP_SERVER_H__

// STD
#include <cstdint>
#include <memory>

// MIF
#include "mif/application/net_base_application.h"
#include "mif/net/http/server.h"

namespace Mif
{
    namespace Application
    {

        class HttpServer
            : public NetBaseApplication
        {
        public:
            HttpServer(int argc, char const **argv,
                    Net::Http::Methods const &methods = {
                            Net::Http::Method::Type::Get,
                            Net::Http::Method::Type::Post
                        }
                    );

        protected:
            virtual void Init(Net::Http::ServerHandlers &handlers);
            virtual void Done();

        private:
            std::size_t m_headersLimit;
            std::size_t m_bodyLimit;
            std::size_t m_chunkSize;
            std::size_t m_pipelineLimit;

            Net::Http::Methods m_methods;
            std::unique_ptr<Net::Http::Server> m_server;

            // NetBaseApplication
            virtual void OnInit() override final;
            virtual void OnDone() override final;
        };

    }   // namespace Application
}   // namespace Mif

#endif  // !__MIF_APPLICATION_HTTP_SERVER_H__
