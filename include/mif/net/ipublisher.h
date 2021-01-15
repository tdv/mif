//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_IPUBLISHER_H__
#define __MIF_NET_IPUBLISHER_H__

// MIF
#include "mif/common/types.h"

namespace Mif
{
    namespace Net
    {

        struct IPublisher
        {
            virtual ~IPublisher() = default;
            virtual void Publish(Common::Buffer buffer) = 0;
        };


    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_IPUBLISHER_H__
