//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_SERVER_H__
#define __MIF_NET_HTTP_SERVER_H__

// STD
#include <cstdint>
#include <memory>
#include <string>

// MIF
#include "mif/net/http/methods.h"
#include "mif/net/http/request_handler.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            class Server final
            {
            public:
                Server(std::string const &host, std::string const &port,
                    std::uint16_t workers,
                    Methods const &allowedMethods,
                    ServerHandlers const &handlers,
                    std::size_t headersSize = -1,
                    std::size_t bodySize = -1,
                    std::size_t requestTimeout = -1);

                ~Server();

            private:
                class Impl;
                std::unique_ptr<Impl> m_impl;
            };

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_SERVER_H__
