//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_DETAIL_PS_H__
#define __MIF_REMOTE_DETAIL_PS_H__

// STD
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

// MIF
#include "mif/common/detail/hierarchy.h"
#include "mif/common/log.h"
#include "mif/common/types.h"
#include "mif/common/uuid_generator.h"
#include "mif/remote/detail/iobject_manager.h"
#include "mif/service/inherited_list.h"
#include "mif/service/iservice.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Remote
    {
        namespace Detail
        {

            using FakeHierarchy = Common::Detail::MakeHierarchy<100>;

            namespace Registry
            {
                namespace Counter
                {
                    inline constexpr std::size_t GetLast(...)
                    {
                        return 0;
                    }

                }   // namespace Counter

                template <typename TInterface>
                struct Registry;

                template <std::size_t I>
                struct Item;

            }   // namespace Registry

            template <typename T>
            inline constexpr bool TIsTservicePtr(Service::TIntrusivePtr<T> const *)
            {
                static_assert(std::is_same<Service::TServicePtr<T>, boost::intrusive_ptr<T>>::value,
                    "TServicePtr should be parameterized by a type derived from IService or be IService.");
                return true;
            }

            inline constexpr bool TIsTservicePtr(...)
            {
                return false;
            }

            template <typename T>
            inline constexpr bool IsTServicePtr()
            {
                return TIsTservicePtr(static_cast<T const *>(nullptr));
            }

            template <typename TResult, typename TSerializer>
            struct FunctionWrap
            {
                using Serializer = typename TSerializer::Serializer;

                template <typename TFunc, typename TStubCreator>
                static void Call(TFunc func, Serializer &serializer, TStubCreator &stubCreator)
                {
                    PackResult(func(), serializer, stubCreator);
                }

            private:
                template <typename T, typename TStubCreator>
                static typename std::enable_if
                    <
                        !std::is_pointer<T>::value &&
                        !std::is_reference<T>::value &&
                        !IsTServicePtr<T>(),
                        void
                    >::type
                PackResult(T && res, Serializer &serializer, TStubCreator &)
                {
                    serializer.PutParams(std::forward<T>(res));
                }

                template <typename T, typename TStubCreator>
                static typename std::enable_if
                    <
                        IsTServicePtr<T>(),
                        void
                    >::type
                PackResult(T res, Serializer &serializer, TStubCreator &stubCreator)
                {
                    using PSType = typename Registry::Registry<typename T::element_type>::template Type<TSerializer>;
                    auto instanceId = stubCreator(res, std::string{PSType::InterfaceId});
                    serializer.PutParams(std::move(instanceId));
                }

                template <typename T, typename TStubCreator>
                static typename std::enable_if
                    <
                        std::is_pointer<T>::value ||
                        std::is_reference<T>::value,
                        void
                    >::type
                PackResult(T &&, typename TSerializer::Serializer &, TStubCreator &)
                {
                    static_assert(!std::is_pointer<T>::value && !std::is_reference<T>::value,
                        "You can't return a pointer or a reference from some interface method. Only value.");
                }
            };

            template <typename TSerializer>
            struct FunctionWrap<void, TSerializer>
            {
                using Serializer = typename TSerializer::Serializer;

                template <typename TFunc, typename TStubCreator>
                static void Call(TFunc func, Serializer &, TStubCreator &)
                {
                    func();
                }
            };

            class ProxyStubException final
                : public std::runtime_error
            {
            public:
                using std::runtime_error::runtime_error;
            };

            template <typename TSerializer>
            class Proxy
            {
            public:
                using Serializer = typename TSerializer::Serializer;
                using Deserializer = typename TSerializer::Deserializer;
                using DeserializerPtr = std::unique_ptr<Deserializer>;

                using Sender = std::function<DeserializerPtr (std::string const &, Serializer &)>;

                Proxy(IObjectManagerPtr manager, std::string const &serviceId, std::string const &interfaceId, Sender && sender)
                    : m_manager{manager}
                    , m_instance{m_manager->CreateObject(serviceId, interfaceId)}
                    , m_sender{std::move(sender)}
                {
                }

                Proxy(IObjectManagerPtr manager, std::string const &instance, Sender && sender)
                    : m_manager{manager}
                    , m_instance{instance}
                    , m_sender{std::move(sender)}
                {
                }

                Proxy(std::string const &instance, Sender && sender)
                    : m_instance{instance}
                    , m_sender{std::move(sender)}
                {
                }

                virtual ~Proxy()
                {
                    if (m_manager)
                    {
                        try
                        {
                            m_manager->DestroyObject(m_instance);
                        }
                        catch (std::exception const &e)
                        {
                            MIF_LOG(Warning) << "[Mif::Remote::Detail::Proxy::~Proxy] "
                                << "Failed to destroy service with instance id "
                                << "\"" << m_instance + "\". Error: " << e.what();
                        }
                    }
                }

                template <typename TResult, typename ... TParams>
                TResult RemoteCall(std::string const &interface, std::string const &method, TParams && ... params)
                {
                    try
                    {
                        auto const requestId = m_generator.Generate();
                        Serializer serializer(true, requestId, m_instance, interface, method, std::forward<TParams>(params) ... );
                        auto deserializer = m_sender(requestId, serializer);
                        if (!deserializer->IsResponse())
                            throw ProxyStubException{"[Mif::Remote::Proxy::RemoteCall] Bad response type \"" + deserializer->GetType() + "\""};
                        auto const &instance = deserializer->GetInstance();
                        if (instance != m_instance)
                        {
                            throw ProxyStubException{"[Mif::Remote::Proxy::RemoteCall] Bad instance id \"" + instance + "\" "
                                "Needed instance id \"" + m_instance + "\""};
                        }
                        auto const &interfaceId = deserializer->GetInterface();
                        if (interface != interfaceId)
                        {
                            throw ProxyStubException{"[Mif::Remote::Proxy::RemoteCall] Bad interface for instance whith id \"" +
                                interfaceId + "\" Needed \"" + interface + "\""};
                        }
                        auto const &methodId = deserializer->GetMethod();
                        if (method != methodId)
                        {
                            throw ProxyStubException{"[Mif::Remote::Proxy::RemoteCall] Method \"" + methodId + "\" "
                                "of interface \"" + interface + "\" for instance with id \"" + m_instance + "\" "
                                "not found. Needed method \"" + method + "\""};
                        }

                        if (deserializer->HasException())
                            std::rethrow_exception(deserializer->GetException());

                        return ExtractResult<TResult>(*deserializer);
                    }
                    catch (std::exception const &e)
                    {
                        throw ProxyStubException{"[Mif::Remote::Proxy::RemoteCall] Failed to call remote method \"" +
                            interface + "::" + method + "\" for instance with id \"" + m_instance + "\". Error: " +
                            std::string{e.what()}};
                    }
                }

                bool QueryRemoteInterface(void **service, std::type_info const &typeInfo, std::string const &serviceId)
                {
                    return CreateProxy<0>(service, std::type_index{typeInfo}, serviceId);
                }

            private:
                Common::UuidGenerator m_generator;
                IObjectManagerPtr m_manager;
                std::string m_instance;
                Sender m_sender;

                template <typename TResult>
                typename std::enable_if
                    <
                        !std::is_same<TResult, void>::value &&
                        !std::is_pointer<TResult>::value &&
                        !std::is_reference<TResult>::value &&
                        !IsTServicePtr<TResult>(),
                        TResult
                    >::type
                ExtractResult(Deserializer &deserializer)
                {
                    return std::get<0>(deserializer.template GetParams<TResult>());
                }

                template <typename TResult>
                typename std::enable_if
                    <
                        IsTServicePtr<TResult>(),
                        TResult
                    >::type
                ExtractResult(Deserializer &deserializer)
                {
                    auto const instanceId = std::get<0>(deserializer.template GetParams<std::string>());
                    if (instanceId.empty())
                        return {};

                    using InterfaceType = typename TResult::element_type;

                    using PSType = typename Registry::Registry<InterfaceType>::template Type<TSerializer>;
                    using ProxyType = typename PSType::Proxy;

                    Sender sender{m_sender};

                    return Service::Make<ProxyType, InterfaceType>(m_manager, instanceId, std::move(sender));
                }

                template <typename TResult>
                typename std::enable_if
                    <
                        std::is_pointer<TResult>::value ||
                        std::is_reference<TResult>::value,
                        TResult
                    >::type
                ExtractResult(Deserializer &)
                {
                    static_assert(!std::is_pointer<TResult>::value && !std::is_reference<TResult>::value,
                        "You can't return a pointer or a reference from some interface method. Only value.");

                    throw std::exception{};
                }

                template <typename TResult>
                typename std::enable_if
                    <
                        std::is_same<TResult, void>::value,
                        TResult
                    >::type
                ExtractResult(Deserializer &de)
                {
                }

                template <std::size_t I>
                bool CreateProxy(typename Registry::template Item<I>::Index::value_type,
                        void **service, std::type_index const &typeId, std::string const &serviceId)
                {
                    using PSType = typename Registry::template Item<I>::Type::template Type<TSerializer>;
                    using InterfaceType = typename PSType::InterfaceType;
                    using ProxyType = typename PSType::Proxy;
                    if (std::type_index{typeid(InterfaceType)} == typeId)
                    {
                        auto const instanceId = m_manager->QueryInterface(m_instance, PSType::InterfaceId, serviceId);
                        if (instanceId.empty())
                            return false;
                        Sender sender{m_sender};
                        auto procy = Service::Make<ProxyType, InterfaceType>(m_manager, instanceId, std::move(sender));
                        // TODO: fix memory leak
                        procy->AddRef();
                        *service = procy.get();
                        return true;
                    }

                    return false;
                }

                template <std::size_t I>
                bool CreateProxy(...)
                {
                    return false;
                }

                template <std::size_t I>
                typename std::enable_if<I == 100, bool>::type
                CreateProxy(void **, std::type_index const &, std::string const &)
                {
                    return false;
                }

                template <std::size_t I>
                typename std::enable_if<I != 100, bool>::type
                CreateProxy(void **service, std::type_index const &typeId, std::string const &serviceId)
                {
                    if (CreateProxy<I>(std::size_t{}, service, typeId, serviceId))
                        return true;
                    return CreateProxy<I + 1>(service, typeId, serviceId);
                }
            };

            template <typename TSerializer>
            struct IStub
            {
                using Serializer = typename TSerializer::Serializer;
                using Deserializer = typename TSerializer::Deserializer;

                virtual ~IStub() = default;
                virtual void Call(Deserializer &request, Serializer &response) = 0;
                virtual Service::IServicePtr Query(std::string const &interfaceId, std::string const &serviceId) = 0;
            };

            template <typename TSerializer>
            class Stub
                : public IStub<TSerializer>
            {
            public:
                using BaseType = IStub<TSerializer>;
                using Serializer = typename BaseType::Serializer;
                using Deserializer = typename BaseType::Deserializer;

                using StubCreator = std::function<std::string (Service::IServicePtr, std::string const &)>;

                Stub(Service::IServicePtr instance, std::string const &instanceId, StubCreator && stubCreator)
                    : m_instance{instance}
                    , m_instanceId{instanceId}
                    , m_stubCreator(std::move(stubCreator))
                {
                }

                virtual void Call(Deserializer &request, Serializer &response) override final
                {
                    try
                    {
                        auto const &interfaceId = request.GetInterface();
                        if (!ContainInterfaceId(interfaceId))
                        {
                            throw ProxyStubException{"[Mif::Remote::Stub::Call] Interface with id \"" +
                                interfaceId + "\" not supported for this object."};
                        }
                        auto const &method = request.GetMethod();
                        InvokeMethod(method, request, response);
                    }
                    catch (...)
                    {
                        response.PutException(std::current_exception());
                    }
                }

                virtual Service::IServicePtr Query(std::string const &interfaceId, std::string const &serviceId) override final
                {
                    return QueryInterface<0>(interfaceId, serviceId);
                }

                template <std::size_t I>
                Service::IServicePtr QueryInterface(typename Registry::template Item<I>::Index::value_type,
                        std::string const &interfaceId, std::string const &serviceId)
                {
                    using PSType = typename Registry::template Item<I>::Type::template Type<TSerializer>;
                    using InterfaceType = typename PSType::InterfaceType;
                    if (interfaceId == PSType::InterfaceId)
                    {
                        auto instance = m_instance->Query<InterfaceType>(serviceId);
                        if (!instance)
                            return {};
                        auto result = instance->template Cast<Service::IService>();
                        return result;
                    }

                    return {};
                }

                template <std::size_t I>
                Service::IServicePtr QueryInterface(...)
                {
                    return {};
                }

                template <std::size_t I>
                typename std::enable_if<I == 100, Service::IServicePtr>::type
                QueryInterface(std::string const &, std::string const &)
                {
                    return {};
                }

                template <std::size_t I>
                typename std::enable_if<I != 100, Service::IServicePtr>::type
                QueryInterface(std::string const &interfaceId, std::string const &serviceId)
                {
                    if (auto instance = QueryInterface<I>(std::size_t{}, interfaceId, serviceId))
                        return instance;
                    return QueryInterface<I + 1>(interfaceId, serviceId);
                }

            private:
                Service::IServicePtr m_instance;
                std::string m_instanceId;
                StubCreator m_stubCreator;

            protected:
                virtual void InvokeMethod(std::string const &method, Deserializer &, Serializer &)
                {
                    throw ProxyStubException{"[Mif::Remote::Stub::InvokeMethod] Method \"" + method + "\" not found."};
                }

                template <typename TResult, typename TInterface, typename ... TParams>
                void InvokeRealMethod(TResult (*method)(TInterface &, std::tuple<TParams ... > && ),
                                      Deserializer &deserializer, Serializer &serializer)
                {
                    auto inst = Service::Cast<TInterface>(m_instance);
                    auto params = deserializer.template GetParams<TParams ... >();
                    FunctionWrap<TResult, TSerializer>::Call(
                            [&method, &inst, &params] ()
                            {
                                return method(*inst, std::move(params));
                            },
                            serializer,
                            m_stubCreator
                        );
                }

                virtual bool ContainInterfaceId(std::string const &id) const
                {
                    return false;
                }
            };

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
                        std::type_info const &typeInfo, std::string const &serviceId) override final
                {
                    return m_proxy.QueryRemoteInterface(service, typeInfo, serviceId);
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


#endif  // !__MIF_REMOTE_DETAIL_PS_H__
