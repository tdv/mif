//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     06.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_EXTERNAL_H__
#define __MIF_SERVICE_EXTERNAL_H__

// STD
#include <string>

// MIF
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Service
    {

        template <typename T>
        class External
            : public Inherit<IService>
            , private Detail::IProxyBase_Mif_Remote_
        {
        public:
            /*External(TServicePtr<T> service, std::string const &servicdId = {})
                : m_service{std::move(service)}
                , m_serviceId{serviceId}
            {
            }*/

        private:
            template <typename>
            friend class Detail::Service_Impl__;

            TServicePtr<T> m_service;
            std::string m_serviceId;

            // IProxyBase_Mif_Remote_
            virtual bool _Mif_Remote_QueryRemoteInterface(void **service,
                    std::type_info const &typeInfo, std::string const &serviceId,
                    IService **holder) override final
            {
                if (!m_service)
                    return false;
                if (typeInfo == typeid(T) && serviceId == m_serviceId)
                {
                    *service = m_service.get();
                    if (holder)
                    {
                        *holder = this;
                        (*holder)->AddRef();
                    }
                    return true;
                }
                return false;
            }
        };

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_EXTERNAL_H__
