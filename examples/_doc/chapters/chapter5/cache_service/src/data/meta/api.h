#ifndef __CACHESERVICE_DATA_META_API_H__
#define __CACHESERVICE_DATA_META_API_H__

// MIF
#include <mif/reflection/reflect_type.h>

// THIS
#include "data/api.h"

namespace CacheService
{
    namespace Data
    {
        namespace Api
        {
            namespace Response
            {
                namespace Meta
                {

                    MIF_REFLECT_BEGIN(Status)
                        MIF_REFLECT_FIELD(code)
                        MIF_REFLECT_FIELD(message)
                    MIF_REFLECT_END()

                    MIF_REFLECT_BEGIN(Info)
                        MIF_REFLECT_FIELD(service)
                        MIF_REFLECT_FIELD(node)
                        MIF_REFLECT_FIELD(timestamp)
                        MIF_REFLECT_FIELD(status)
                    MIF_REFLECT_END()

                    MIF_REFLECT_BEGIN(Header)
                        MIF_REFLECT_FIELD(meta)
                    MIF_REFLECT_END()

                    MIF_REFLECT_BEGIN(Data)
                        MIF_REFLECT_FIELD(meta)
                        MIF_REFLECT_FIELD(data)
                    MIF_REFLECT_END()

                    MIF_REFLECT_BEGIN(BucketKeys)
                        MIF_REFLECT_FIELD(meta)
                        MIF_REFLECT_FIELD(data)
                    MIF_REFLECT_END()

                }   // namespace Meta
            }   // namespace Response
        }   // namespace Api
    }   // namespace Data
}   // namespace CacheService

MIF_REGISTER_REFLECTED_TYPE(CacheService::Data::Api::Response::Meta::Status)
MIF_REGISTER_REFLECTED_TYPE(CacheService::Data::Api::Response::Meta::Info)
MIF_REGISTER_REFLECTED_TYPE(CacheService::Data::Api::Response::Meta::Header)
MIF_REGISTER_REFLECTED_TYPE(CacheService::Data::Api::Response::Meta::Data)
MIF_REGISTER_REFLECTED_TYPE(CacheService::Data::Api::Response::Meta::BucketKeys)

#endif  // !__CACHESERVICE_DATA_META_API_H__
