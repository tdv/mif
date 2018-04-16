#ifndef __CACHESERVICE_INTERFACE_IAUTH_H__
#define __CACHESERVICE_INTERFACE_IAUTH_H__

// MIF
#include <mif/service/iservice.h>

namespace CacheService
{

    struct IAuth
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
    };

    using IAuthPtr = Mif::Service::TServicePtr<IAuth>;

}   // namespace CacheService

#endif  // !__CACHESERVICE_INTERFACE_IAUTH_H__
