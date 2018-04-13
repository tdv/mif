// MIF
#include <mif/service/creator.h>

// THIS
#include "id/service.h"

namespace CacheService
{
    namespace Detail
    {
        namespace
        {

            class DataFacace
                : public Mif::Service::Inherit<Mif::Service::IService>
            {
            public:
            private:
            };

        }   // namespace
    }   // namespace Detail
}   // namespace CacheService

MIF_SERVICE_CREATOR
(
    ::CacheService::Id::DataFacade,
    ::CacheService::Detail::DataFacace
)
