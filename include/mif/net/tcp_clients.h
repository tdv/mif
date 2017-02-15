//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_TCP_CLIENTS_H__
#define __MIF_NET_TCP_CLIENTS_H__

// STD
#include <cstdint>
#include <memory>
#include <string>

// MIF
#include "mif/net/iclient_factory.h"

namespace Mif
{
    namespace Net
    {

        class TCPClients final
        {
        public:
            TCPClients(IClientFactoryPtr factory);
            ~TCPClients();

            IClientFactory::ClientPtr RunClient(std::string const &host, std::string const &port);

        private:
            class Impl;
            std::unique_ptr<Impl> m_impl;
        };

    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_TCP_CLIENTS_H__
