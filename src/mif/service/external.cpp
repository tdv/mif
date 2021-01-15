//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     06.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

// MIF
#include "mif/service/external.h"

namespace Mif
{
    namespace Service
    {

    void External::Add(IServicePtr service)
    {
        LockGuard lock{m_lock};
        m_services.emplace_back(service);
    }

    bool External::_Mif_Remote_QueryRemoteInterface(void **service,
            std::type_info const &typeInfo, std::string const &serviceId,
            IService **holder)
        {
            LockGuard lock{m_lock};

            if (m_services.empty())
                return false;

            for (auto &i : m_services)
            {
                bool res = i->Query(typeInfo, service, serviceId, holder);
                if (res)
                    return true;
            }

            return false;
        }

    }   // namespace Service
}   // namespace Mif
