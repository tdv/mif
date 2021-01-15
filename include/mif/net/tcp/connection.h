//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_TCP_CONNECTION_H__
#define __MIF_NET_TCP_CONNECTION_H__

// STD
#include <memory>
#include <string>

// MIF
#include "mif/net/iclient_factory.h"
#include "mif/net/iconnection.h"

namespace Mif
{
    namespace Net
    {
        namespace Tcp
        {

            class Connection final
                : public IConnection
            {
            public:
                Connection(std::string const &host, std::string const &port,
                        IClientFactoryPtr factory);

                ~Connection();

                // IConnection
                virtual ClientPtr GetClient() override final;

            private:
                class Impl;
                std::unique_ptr<Impl> m_impl;
            };

        }   // namespace Tcp
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_TCP_CLIENTS_H__
