//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     05.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_IPOOL_H__
#define __MIF_SERVICE_IPOOL_H__

// MIF
#include "mif/service/iservice.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Service
    {

        namespace Detail
        {

            class Pool;

        }   // namespace Detail

        struct IPool
            : public Inherit<IService>
        {
            template <typename T>
            class Proxy
                : public Inherit<IService>
            {
            public:
                TServicePtr<T> Get()
                {
                    return m_service;
                }

                TServicePtr<T> Get() const
                {
                    return m_service;
                }

                TServicePtr<T> operator -> ()
                {
                    return Get();
                }

                TServicePtr<T> operator -> () const
                {
                    return Get();
                }

            private:
                friend struct IPool;
                template <typename>
                friend class Detail::Service_Impl__;

                IServicePtr m_owner;
                TServicePtr<T> m_service;

                Proxy(IServicePtr owner, TServicePtr<T> service)
                    : m_owner{owner}
                    , m_service{service}
                {
                }

                template <typename TOther>
                Proxy<TOther> Cast()
                {
                    return Make<Proxy<TOther>>(m_owner, Service::Cast<TOther>(m_service));
                }
            };

            using ProxyPtr = TServicePtr<Proxy<IService>>;

            virtual ProxyPtr GetService() const = 0;

            template <typename T>
            Proxy<T> GetService() const
            {
                return GetService()->Cast<T>();
            }
        };

        using IPoolPtr = Service::TServicePtr<IPool>;

    }   // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_IPOOL_H__
