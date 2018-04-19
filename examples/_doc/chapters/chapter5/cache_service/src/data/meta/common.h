#ifndef __CACHESERVICE_DATA_META_COMMON_H__
#define __CACHESERVICE_DATA_META_COMMON_H__

// MIF
#include <mif/reflection/reflect_type.h>

// THIS
#include "data/common.h"

namespace CacheService
{
    namespace Data
    {
        namespace Meta
        {

            MIF_REFLECT_BEGIN(Role)
                MIF_REFLECT_FIELD(Admin)
                MIF_REFLECT_FIELD(User)
            MIF_REFLECT_END()

            MIF_REFLECT_BEGIN(Profile)
                MIF_REFLECT_FIELD(login)
                MIF_REFLECT_FIELD(password)
                MIF_REFLECT_FIELD(name)
                MIF_REFLECT_FIELD(roles)
                MIF_REFLECT_FIELD(buckets)
            MIF_REFLECT_END()

        }   // namespace Meta
    }   // namespace Data
}   // namespace CacheService

MIF_REGISTER_REFLECTED_TYPE(CacheService::Data::Meta::Role)
MIF_REGISTER_REFLECTED_TYPE(CacheService::Data::Meta::Profile)

#endif  // !__CACHESERVICE_DATA_META_COMMON_H__
