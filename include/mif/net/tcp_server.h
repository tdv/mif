//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
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

        class TCPServer final
        {
        public:
            TCPServer(std::string const &host, std::string const &port,
                std::uint16_t workers, std::shared_ptr<IClientFactory> factory);

            ~TCPServer();

        private:
            class Impl;
            std::unique_ptr<Impl> m_impl;
        };

    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_TCP_SERVER_H__
