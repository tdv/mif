//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_CLIENTS_GZIP_DECOMPRESSOR_H__
#define __MIF_NET_CLIENTS_GZIP_DECOMPRESSOR_H__

// STD
#include <memory>

// MIF
#include "mif/net/client.h"

namespace Mif
{
    namespace Net
    {
        namespace Clients
        {

            class GZipDecompressor final
                : public Net::Client
            {
            public:
                GZipDecompressor(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher);

            protected:
                // Client
                virtual void ProcessData(Common::Buffer buffer) override final;
            };

        }   // namespace Clients
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_CLIENTS_GZIP_DECOMPRESSOR_H__
