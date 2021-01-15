//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_DETAIL_PS_BASE_H__
#define __MIF_REMOTE_DETAIL_PS_BASE_H__

// STD
#include <string>
#include <tuple>
#include <typeinfo>
#include <utility>

// MIF
#include "mif/remote/detail/ps.h"
#include "mif/remote/detail/registry.h"
#include "mif/service/inherited_list.h"
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Remote
    {
        namespace Detail
        {

            template <typename, typename, typename>
            class BaseProxies;

            template <typename TSerializer, typename TInterface, typename TBase, typename ... TBases>
            class BaseProxies<TSerializer, TInterface, std::tuple<TBase, TBases ... >>
                : public Registry::Registry<TBase>::template Type<TSerializer>::template ProxyItem
                    <
                        BaseProxies<TSerializer, TInterface, std::tuple<TBases ... >>
                    >
            {
            protected:
                using Registry::Registry<TBase>::template Type<TSerializer>::template ProxyItem
                        <
                            BaseProxies<TSerializer, TInterface, std::tuple<TBases ... >>
                        >::ProxyItem;

                virtual ~BaseProxies() = default;
            };

            template <typename TSerializer, typename TInterface>
            class BaseProxies<TSerializer, TInterface, std::tuple<>>
                : public Service::Inherit<TInterface>
                , public Service::Detail::IProxyBase_Mif_Remote_
            {
            protected:
                template <typename ... TParams>
                BaseProxies(TParams && ... params)
                    : m_proxy(std::forward<TParams>(params) ... )
                {
                }

                virtual ~BaseProxies() = default;

                template <typename TResult, typename ... TParams>
                TResult _Mif_Remote_Call_Method(std::string const &interfaceId, std::string const &method, TParams && ... params) const
                {
                    return m_proxy.template RemoteCall<TResult>(interfaceId, method, std::forward<TParams>(params) ... );
                }

            private:
                mutable Proxy<TSerializer> m_proxy;

                // IProxyBase_Mif_Remote_
                virtual bool _Mif_Remote_QueryRemoteInterface(void **service,
                        std::type_info const &typeInfo, std::string const &serviceId,
                        Service::IService **holder) override final
                {
                    return m_proxy.QueryRemoteInterface(service, typeInfo, serviceId, holder);
                }
            };

            template <typename TSerializer, typename T>
            using InheritProxy = BaseProxies<TSerializer, T, Service::MakeInheritedIist<T>>;

            template <typename, typename>
            class BaseStubs;

            template <typename TSerializer, typename TBase, typename ... TBases>
            class BaseStubs<TSerializer, std::tuple<TBase, TBases ... >>
                : public Registry::Registry<TBase>::template Type<TSerializer>::template StubItem
                    <
                        BaseStubs<TSerializer, std::tuple<TBases ... >>
                    >
            {
            protected:
                using BaseType = typename Registry::Registry<TBase>::template Type<TSerializer>::template StubItem
                        <
                            BaseStubs<TSerializer, std::tuple<TBases ... >>
                        >;
                using Serializer = typename BaseType::Serializer;
                using Deserializer = typename BaseType::Deserializer;

                using BaseType::StubItem;

                virtual ~BaseStubs() = default;
            };

            template <typename TSerializer>
            class BaseStubs<TSerializer, std::tuple<>>
                : public ::Mif::Remote::Detail::Stub<TSerializer>
            {
            protected:
                using BaseType = ::Mif::Remote::Detail::Stub<TSerializer>;
                using Serializer = typename BaseType::Serializer;
                using Deserializer = typename BaseType::Deserializer;

                using BaseType::Stub;

                virtual ~BaseStubs() = default;
            };

            template <typename TSerializer, typename T>
            class InheritStub
                : public BaseStubs<TSerializer, Service::MakeInheritedIist<T>>
            {
            protected:
                using BaseType = BaseStubs<TSerializer, Service::MakeInheritedIist<T>>;
                using Serializer = typename BaseType::Serializer;
                using Deserializer = typename BaseType::Deserializer;

                using BaseType::BaseStubs;

                virtual ~InheritStub() = default;
            };

        }  // namespace Detail
    }   //  namespace Remote
}   // namespace Mif


#endif  // !__MIF_REMOTE_DETAIL_PS_BASE_H__
