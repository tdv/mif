//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __COMPLEX_TYPE_COMMON_ID_SERVICE_H__
#define __COMPLEX_TYPE_COMMON_ID_SERVICE_H__

// MIF
#include <mif/common/crc32.h>

namespace Service
{
    namespace Id
    {

        enum
        {
            MyCompany = Mif::Common::Crc32("MyCompany")
        };

    }   // namespace Id
}   // namespace Service

#endif  // !__COMPLEX_TYPE_COMMON_ID_SERVICE_H__
