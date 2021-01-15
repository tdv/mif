//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_ICONNECTION_H__
#define __MIF_NET_ICONNECTION_H__

// STD
#include <memory>

// MIF
#include "mif/net/client.h"

namespace Mif
{
    namespace Net
    {

        struct IConnection
        {
            virtual ~IConnection() = default;

            using ClientPtr = std::shared_ptr<Client>;

            virtual ClientPtr GetClient() = 0;
        };

    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_ICONNECTION_H__
