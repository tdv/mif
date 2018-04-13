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

                class Auth
                    : public Base
                {
                public:
                    Auth(std::string const &prefix)
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
    ::CacheService::Id::Auth,
    ::CacheService::Handler::Detail::Auth,
    std::string
)
