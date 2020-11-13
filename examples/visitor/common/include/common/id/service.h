//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __VISITOR_COMMON_ID_SERVICE_H__
#define __VISITOR_COMMON_ID_SERVICE_H__

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

#endif  // !__VISITOR_COMMON_ID_SERVICE_H__
