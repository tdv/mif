//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_TCP_SERVER_H__
#define __MIF_NET_TCP_SERVER_H__

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
        namespace Tcp
        {

            class Server final
            {
            public:
                Server(std::string const &host, std::string const &port, IClientFactoryPtr factory);

                ~Server();

            private:
                class Impl;
                std::unique_ptr<Impl> m_impl;
            };

        }   // namespace Tcp
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_TCP_SERVER_H__
