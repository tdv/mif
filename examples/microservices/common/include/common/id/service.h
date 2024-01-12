//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MICROSERVICES_COMMON_ID_SERVICE_H__
#define __MICROSERVICES_COMMON_ID_SERVICE_H__

// MIF
#include <mif/common/crc32.h>

namespace Common
{
    namespace Service
    {
        namespace Id
        {

            enum
            {
                PGEmployeeStorage = Mif::Common::Crc32("EmployeeStorage.PostgreSQL")
            };

        }   // namespace Id
    }   // namespace Service
}   // namespace Common

#endif  // !__MICROSERVICES_COMMON_ID_SERVICE_H__
