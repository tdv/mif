// STD
#include <map>
#include <mutex>

// MIF
#include <mif/service/creator.h>

// THIS
#include "exception.h"
#include "id/service.h"
#include "interface/iadmin.h"
#include "interface/iauth.h"
#include "interface/icache.h"

namespace CacheService
{
    namespace Detail
    {
        namespace
        {

            class DataFacace
                : public Mif::Service::Inherit
                    <
                        IAuth, IAdmin, ICache
                    >
            {
            public:

            private:
                using LockType = std::mutex;
                using LockGuard = std::lock_guard<LockType>;

                using Cache = std::map<std::pair<Data::ID, Data::ID>, std::string>;

                mutable LockType m_lock;
                Cache m_cache;

                //IAuth

                //IAdmin

                // ICache
                virtual std::map<Data::ID, Data::IDs> ListBucketKeys(Data::IDs const &buckets) const override final
                {
                    std::map<Data::ID, Data::IDs> ret;

                    if (buckets.empty())
                    {
                        LockGuard lock{m_lock};
                        for (auto const &i : m_cache)
                            ret[i.first.first].insert(i.first.second);
                    }
                    else
                    {
                        LockGuard lock{m_lock};
                        for (auto const &i : m_cache)
                        {
                            if (buckets.find(i.first.first) != std::end(buckets))
                                ret[i.first.first].insert(i.first.second);
                        }
                    }

                    return ret;
                }

                virtual std::string GetData(Data::ID const &bucket, Data::ID const &key) const override final
                {
                    std::string data;

                    {
                        LockGuard lock{m_lock};
                        auto const iter = m_cache.find(std::make_pair(bucket, key));
                        if (iter == std::end(m_cache))
                            throw Exception::NotFound{"Data for bucket \"" + bucket + "\" and key \"" + key + "\" not found."};
                        data = iter->second;
                    }

                    return data;
                }

                virtual void SetData(Data::ID const &bucket, Data::ID const &key,
                        std::string const &data) override final
                {
                    LockGuard lock{m_lock};
                    m_cache[std::make_pair(bucket, key)] = data;
                }

                virtual void RemoveData(Data::ID const &bucket, Data::ID const &key) override final
                {
                    LockGuard lock{m_lock};
                    auto iter = m_cache.find(std::make_pair(bucket, key));
                    if (iter == std::end(m_cache))
                        throw Exception::NotFound{"Data for bucket \"" + bucket + "\" and key \"" + key + "\" not found."};
                    m_cache.erase(iter);
                }
            };

        }   // namespace
    }   // namespace Detail
}   // namespace CacheService

MIF_SERVICE_CREATOR
(
    ::CacheService::Id::DataFacade,
    ::CacheService::Detail::DataFacace
)
