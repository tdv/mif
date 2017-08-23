//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MICROSERVICES_SERVICE_ID_SERVICE_H__
#define __MICROSERVICES_SERVICE_ID_SERVICE_H__

// MIF
#include <mif/common/crc32.h>

namespace Service
{
    namespace Id
    {

        enum
        {
            Employee = Mif::Common::Crc32("Employee.Http")
        };

    }   // namespace Id
}   // namespace Service

#endif  // !__MICROSERVICES_SERVICE_ID_SERVICE_H__
