#ifndef __CACHESERVICE_INTERFACE_IAUTH_H__
#define __CACHESERVICE_INTERFACE_IAUTH_H__

// MIF
#include <mif/service/iservice.h>

// THIS
#include "data/common.h"

namespace CacheService
{

    struct IAuth
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
        virtual Data::Session Login(std::string const &login, std::string const &password) = 0;
        virtual void Logout(Data::ID const &session) = 0;
        virtual Data::Profile GetSessionProfile(Data::ID const &session) const = 0;
    };

    using IAuthPtr = Mif::Service::TServicePtr<IAuth>;

}   // namespace CacheService

#endif  // !__CACHESERVICE_INTERFACE_IAUTH_H__
