// MIF
#include <mif/service/creator.h>

// THIS
#include "base.h"
#include "id/service.h"

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
                        return std::string{};
                    }

                    Response Get()
                    {
                        return std::string{};
                    }

                    Response Set()
                    {
                        return std::string{};
                    }

                    Response Remove()
                    {
                        return std::string{};
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
