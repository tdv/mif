//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016 tdv
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

        class HTTPClients final
        {
        public:
            HTTPClients(std::shared_ptr<IClientFactory> factory);
            ~HTTPClients();

            IClientFactory::ClientPtr RunClient(std::string const &host, std::string const &port);

        private:
            class Impl;
            std::unique_ptr<Impl> m_impl;
        };

    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_CLIENTS_H__
