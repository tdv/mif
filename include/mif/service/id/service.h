//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_ID_SERVICE_H__
#define __MIF_SERVICE_ID_SERVICE_H__

// MIF
#include "mif/common/crc32.h"

namespace Mif
{
    namespace Service
    {
        namespace Id
        {

            enum
            {

                Locator = Common::Crc32("Mif.Service.Locator"),
                Pool = Common::Crc32("Mif.Service.Pool"),
                PerThreadPool = Common::Crc32("Mif.Service.PerThreadPool")

            };

        }   // namespace Id
    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_ID_SERVICE_H__
