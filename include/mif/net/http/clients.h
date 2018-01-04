//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_CLIENTS_H__
#define __MIF_NET_HTTP_CLIENTS_H__

// STD
#include <memory>
#include <string>

// MIF
#include "mif/net/iclient_factory.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            class Clients final
            {
            public:
                Clients(std::shared_ptr<IClientFactory> factory);
                ~Clients();

                IClientFactory::ClientPtr RunClient(std::string const &host, std::string const &port,
                        std::string const &resource = "");

            private:
                class Impl;
                std::unique_ptr<Impl> m_impl;
            };

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_CLIENTS_H__
