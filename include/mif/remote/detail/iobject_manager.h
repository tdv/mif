//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_DETAIL_IOBJECT_MANAGER_H__
#define __MIF_REMOTE_DETAIL_IOBJECT_MANAGER_H__

// STD
#include <string>

// MIF
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Remote
    {
        namespace Detail
        {

            struct IObjectManager
                : public Service::Inherit<Service::IService>
            {
                virtual ~IObjectManager() = default;
                virtual std::string CreateObject(Service::ServiceId serviceId, std::string const &interfaceId) = 0;
                virtual void DestroyObject(std::string const &instanceId) = 0;
                virtual std::string QueryInterface(std::string const &instanceId, std::string const &interfaceId,
                        std::string const &serviceId) = 0;
                virtual std::string CloneReference(std::string const &instanceId, std::string const &interfaceId) = 0;
            };

            using IObjectManagerPtr = Service::TServicePtr<IObjectManager>;

        }   // namespace Detail
    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_DETAIL_IOBJECT_MANAGER_H__
