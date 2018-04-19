#ifndef __CACHESERVICE_INTERFACE_IADMIN_H__
#define __CACHESERVICE_INTERFACE_IADMIN_H__

// MIF
#include <mif/service/iservice.h>

// THIS
#include "data/common.h"

namespace CacheService
{

    struct IAdmin
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
        virtual Data::StringMap GetProfilesList() const = 0;
        virtual Data::Profile GetProfile(std::string const &login) const = 0;
        virtual void SetProfile(Data::Profile const &profile) = 0;
        virtual void RemoveProfile(std::string const &login) = 0;
    };

    using IAdminPtr = Mif::Service::TServicePtr<IAdmin>;

}   // namespace CacheService

#endif  // !__CACHESERVICE_INTERFACE_IADMIN_H__
