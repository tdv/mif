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
                : public Service::IService
            {
                virtual ~IObjectManager() = default;
                virtual std::string CreateObject(std::string const &serviceId, std::string const &interfaceId) = 0;
                virtual void DestroyObject(std::string const &instanceId) = 0;
            };

        }   // namespace Detail
    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_DETAIL_IOBJECT_MANAGER_H__
