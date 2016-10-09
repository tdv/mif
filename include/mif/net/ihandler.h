//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_IHANDLER_H__
#define __MIF_NET_IHANDLER_H__

// MIF
#include "mif/common/types.h"

namespace Mif
{
    namespace Net
    {

        struct IHandler
        {
            virtual ~IHandler() = default;
            virtual void OnData(Common::Buffer buffer) = 0;
        };

    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_IHANDLER_H__
