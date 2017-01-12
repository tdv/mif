//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2017 tdv
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

                Locator = Common::Crc32("Mif.Service.Locator")

            };

        }   // namespace Id
    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_ID_SERVICE_H__
