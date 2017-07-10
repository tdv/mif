//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     06.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_EXTERNAL_H__
#define __MIF_SERVICE_EXTERNAL_H__

// STD
#include <list>
#include <mutex>

// MIF
#include "mif/common/unused.h"
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Service
    {

        class External
            : public Inherit<IService>
            , private Detail::IProxyBase_Mif_Remote_
        {
        public:
            void Add(IServicePtr service);

        private:
            template <typename>
            friend class Detail::Service_Impl__;

            using LockType = std::mutex;
            using LockGuard = std::lock_guard<LockType>;

            using Services = std::list<IServicePtr>;

            LockType m_lock;

            Services m_services;

            // IProxyBase_Mif_Remote_
            virtual bool _Mif_Remote_QueryRemoteInterface(void **service,
                    std::type_info const &typeInfo, std::string const &serviceId,
                    IService **holder) override final;
        };

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_EXTERNAL_H__
