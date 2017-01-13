//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __PROTO_STACK_COMMON_ID_SERVICE_H__
#define __PROTO_STACK_COMMON_ID_SERVICE_H__

// MIF
#include <mif/common/crc32.h>

namespace Service
{
    namespace Id
    {

        enum
        {
            Service = Mif::Common::Crc32("Service")
        };

    }   // namespace Id
}   // namespace Service

#endif  // !__PROTO_STACK_COMMON_ID_SERVICE_H__
