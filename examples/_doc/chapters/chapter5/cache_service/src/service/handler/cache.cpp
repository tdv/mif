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
                    Response List()
                    {
                        auto cache = Mif::Service::RootLocator::Get()->Get<ICache>(Id::DataFacade);

                        Data::Api::Response::BucketKeys response;

                        response.meta = GetMeta();
                        response.data = cache->ListBucketKeys({});

                        return response;
                    }

                    Response Get(Prm<Data::ID, Name("bucket")> const &bucket,
                                 Prm<Data::ID, Name("key")> const &key)
                    {
                        CheckId(bucket, key);

                        auto cache = Mif::Service::RootLocator::Get()->Get<ICache>(Id::DataFacade);

                        Data::Api::Response::Data response;

                        response.meta = GetMeta();
                        response.data = cache->GetData(bucket.Get(), key.Get());

                        return response;
                    }

                    Response Set(Prm<Data::ID, Name("bucket")> const &bucket,
                                 Prm<Data::ID, Name("key")> const &key,
                                 Content<std::string> const &data)
                    {
                        CheckId(bucket, key);

                        auto cache = Mif::Service::RootLocator::Get()->Get<ICache>(Id::DataFacade);
                        cache->SetData(bucket.Get(), key.Get(), data.Get());

                        Data::Api::Response::Header response;

                        response.meta = GetMeta();

                        return response;
                    }

                    Response Remove(Prm<Data::ID, Name("bucket")> const &bucket,
                                    Prm<Data::ID, Name("key")> const &key)
                    {
                        CheckId(bucket, key);

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
