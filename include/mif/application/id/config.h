//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_APPLICATION_ID_CONFIG_H__
#define __MIF_APPLICATION_ID_CONFIG_H__

// MIF
#include "mif/common/crc32.h"

namespace Mif
{
    namespace Application
    {
        namespace Id
        {
            namespace Service
            {
                namespace Config
                {

                    enum
                    {
                        Json = Common::Crc32("Mif.Application.Service.Config.Json"),
                        Xml = Common::Crc32("Mif.Application.Service.Config.Xml")
                    };

                }   // namespace Config
            }   // namespace Service
        }   // namespace Id
    }   // namespace Application
}   // namespace Mif

#endif  // !__MIF_APPLICATION_ID_CONFIG_H__
