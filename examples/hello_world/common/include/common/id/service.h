//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __HELLO_WORLD_COMMON_ID_SERVICE_H__
#define __HELLO_WORLD_COMMON_ID_SERVICE_H__

// MIF
#include <mif/common/crc32.h>

namespace Service
{
    namespace Id
    {

        enum
        {
            HelloWorld = Mif::Common::Crc32("HelloWorld")
        };

    }   // namespace Id
}   // namespace Service

#endif  // !__HELLO_WORLD_COMMON_ID_SERVICE_H__
