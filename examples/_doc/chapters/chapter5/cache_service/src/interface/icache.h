#ifndef __CACHESERVICE_INTERFACE_ICACHE_H__
#define __CACHESERVICE_INTERFACE_ICACHE_H__

// STD
#include <map>

// MIF
#include <mif/service/iservice.h>

// THIS
#include "data/common.h"

namespace CacheService
{

    struct ICache
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
        virtual std::map<Data::ID, Data::IDs> ListBucketKeys(Data::IDs const &buckets) const = 0;
        virtual std::string GetData(Data::ID const &bucket, Data::ID const &key) const = 0;
        virtual void SetData(Data::ID const &bucket, Data::ID const &key, std::string const &data) = 0;
        virtual void RemoveData(Data::ID const &bucket, Data::ID const &key) = 0;
    };

    using ICachePtr = Mif::Service::TServicePtr<ICache>;

}   // namespace CacheService

#endif  // !__CACHESERVICE_INTERFACE_ICACHE_H__
