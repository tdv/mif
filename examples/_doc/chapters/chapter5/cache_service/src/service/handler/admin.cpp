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

                class Admin
                    : public Base
                {
                public:
                    Admin(std::string const &prefix)
                        : Base{prefix}
                    {
                    }

                private:
                };

            }   // namespace
        }   // namespace Detail
    }   // namespace Handler
}   // namespace CacheService

MIF_SERVICE_CREATOR
(
    ::CacheService::Id::Admin,
    ::CacheService::Handler::Detail::Admin,
    std::string
)
