// MIF
#include <mif/service/creator.h>
#include <mif/service/root_locator.h>

// THIS
#include "base.h"
#include "data/common.h"
#include "data/meta/api.h"
#include "id/service.h"
#include "interface/icache.h"

namespace CacheService
{
    namespace Handler
    {
        namespace Detail
        {
            namespace
            {

                class Cache
                    : public Base
                {
                public:
                    Cache(std::string const &prefix)
                        : Base{prefix}
                    {
                        AddHandler("/list", this, &Cache::List);
                        AddHandler("/get", this, &Cache::Get);
                        AddHandler("/set", this, &Cache::Set);
                        AddHandler("/remove", this, &Cache::Remove);
                    }

                private:
                    // WebService.Hadlers
                    Response List(Headers const &headers)
                    {
                        auto const profile = CheckPermissions(headers, {Data::Role::User});

                        auto cache = Mif::Service::RootLocator::Get()->Get<ICache>(Id::DataFacade);

                        Data::Api::Response::BucketKeys response;

                        response.meta = GetMeta();
                        response.data = cache->ListBucketKeys(profile.buckets);

                        return response;
                    }

                    Response Get(Headers const &headers,
                                 Prm<Data::ID, Name("bucket")> const &bucket,
                                 Prm<Data::ID, Name("key")> const &key)
                    {
                        auto const profile = CheckPermissions(headers, {Data::Role::User});
                        CheckId(bucket, key);
                        CheckBucketId(bucket.Get(), profile.buckets);

                        auto cache = Mif::Service::RootLocator::Get()->Get<ICache>(Id::DataFacade);

                        Data::Api::Response::Data response;

                        response.meta = GetMeta();
                        response.data = cache->GetData(bucket.Get(), key.Get());

                        return response;
                    }

                    Response Set(Headers const &headers,
                                 Prm<Data::ID, Name("bucket")> const &bucket,
                                 Prm<Data::ID, Name("key")> const &key,
                                 Content<std::string> const &data)
                    {
                        auto const profile = CheckPermissions(headers, {Data::Role::User});
                        CheckId(bucket, key);
                        CheckBucketId(bucket.Get(), profile.buckets);

                        auto cache = Mif::Service::RootLocator::Get()->Get<ICache>(Id::DataFacade);
                        cache->SetData(bucket.Get(), key.Get(), data.Get());

                        Data::Api::Response::Header response;

                        response.meta = GetMeta();

                        return response;
                    }

                    Response Remove(Headers const &headers,
                                    Prm<Data::ID, Name("bucket")> const &bucket,
                                    Prm<Data::ID, Name("key")> const &key)
                    {
                        auto const profile = CheckPermissions(headers, {Data::Role::User});
                        CheckId(bucket, key);
                        CheckBucketId(bucket.Get(), profile.buckets);

                        auto cache = Mif::Service::RootLocator::Get()->Get<ICache>(Id::DataFacade);
                        cache->RemoveData(bucket.Get(), key.Get());

                        Data::Api::Response::Header response;

                        response.meta = GetMeta();

                        return response;
                    }

                    void CheckId(Prm<Data::ID, Name("bucket")> const &bucket,
                                 Prm<Data::ID, Name("key")> const &key) const
                    {
                        if (!bucket)
                            throw std::invalid_argument{"No \"bucket\" parameter."};
                        if (bucket.Get().empty())
                            throw std::invalid_argument{"\"bucket\" must not be empty."};

                        if (!key)
                            throw std::invalid_argument{"No \"key\" parameter."};
                        if (key.Get().empty())
                            throw std::invalid_argument{"\"key\" must not be empty."};
                    }

                    void CheckBucketId(Data::ID const &bucket, Data::IDs const &buckets) const
                    {
                        if (buckets.find(bucket) == std::end(buckets))
                            throw std::invalid_argument{"You can't working with bucket \"" + bucket + "\"."};
                    }
                };

            }   // namespace
        }   // namespace Detail
    }   // namespace Handler
}   // namespace CacheService

MIF_SERVICE_CREATOR
(
    ::CacheService::Id::Cache,
    ::CacheService::Handler::Detail::Cache,
    std::string
)
