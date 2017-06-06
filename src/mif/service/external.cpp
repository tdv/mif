//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     06.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include "mif/service/external.h"

namespace Mif
{
    namespace Service
    {

    void External::Set(IServicePtr service)
    {
        LockGuard lock{m_lock};
        m_service.swap(service);
    }

    bool External::_Mif_Remote_QueryRemoteInterface(void **service,
            std::type_info const &typeInfo, std::string const &serviceId,
            IService **holder)
        {
            LockGuard lock{m_lock};

            if (!m_service)
                return false;

            return m_service->Query(typeInfo, service, serviceId, holder);
        }

    }   // namespace Service
}   // namespace Mif
