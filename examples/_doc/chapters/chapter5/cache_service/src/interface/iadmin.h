#ifndef __CACHESERVICE_INTERFACE_IADMIN_H__
#define __CACHESERVICE_INTERFACE_IADMIN_H__

// MIF
#include <mif/service/iservice.h>

namespace CacheService
{

    struct IAdmin
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
    };

    using IAdminPtr = Mif::Service::TServicePtr<IAdmin>;

}   // namespace CacheService

#endif  // !__CACHESERVICE_INTERFACE_IADMIN_H__
