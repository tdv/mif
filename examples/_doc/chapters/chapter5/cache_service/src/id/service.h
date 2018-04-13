#ifndef __CACHESERVICE_ID_H__
#define __CACHESERVICE_ID_H__

// MIF
#include <mif/common/crc32.h>

namespace CacheService
{
    namespace Id
    {

        enum
        {
            DataFacade = Mif::Common::Crc32("CacheService.Id.DataFacade"),
            Auth = Mif::Common::Crc32("CacheService.Id.Auth"),
            Admin = Mif::Common::Crc32("CacheService.Id.Admin"),
            Cache = Mif::Common::Crc32("CacheService.Id.Cache")
        };

    }   // namespace Id
}   // namespace CacheService

#endif  // !__CACHESERVICE_ID_H__
