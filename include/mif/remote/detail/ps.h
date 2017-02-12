//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_DETAIL_PS_H__
#define __MIF_REMOTE_DETAIL_PS_H__

// STD
#include <algorithm>
#include <functional>
#include <list>
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
#include "mif/common/index_sequence.h"
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

            template <typename T>
            using ExtractType = typename std::remove_const<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>::type;

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

            using StubCreator = std::function<std::string (Service::IServicePtr, std::string const &)>;

            class ObjectCleaner final
            {
            public:
                ObjectCleaner(Service::TIntrusivePtr<IObjectManager> manager)
                    : m_manager{std::move(manager)}
                {
                }

                ObjectCleaner(ObjectCleaner const &) = delete;
                ObjectCleaner& operator = (ObjectCleaner const &) = delete;
                ObjectCleaner(ObjectCleaner &&) = delete;
                ObjectCleaner& operator = (ObjectCleaner &&) = delete;

                ~ObjectCleaner()
                {
                    if (!m_manager)
                        return;

                    std::for_each(std::begin(m_ids), std::end(m_ids),
                            [this] (std::string const &id)
                            {
                                try
                                {
                                    m_manager->DestroyObject(id);
                                }
                                catch (std::exception const &e)
                                {
                                    MIF_LOG(Warning) << "[Mif::Remote::Detail::ObjectCleaner::~ObjectCleaner] "
                                                     << "Failed to call Destroy object for instance whith id \""
                                                     << id << "\" Error: " << e.what();
                                }
                            }
                        );
                }

                void AppendId(std::string const &id)
                {
                    // TODO: uncomment it
                    //m_ids.push_back(id);
                }

            private:
                Service::TIntrusivePtr<IObjectManager> m_manager;
                std::list<std::string> m_ids;
            };

            template <typename TSerializer>
            class Proxy
            {
            public:
                using Serializer = typename TSerializer::Serializer;
                using Deserializer = typename TSerializer::Deserializer;
                using DeserializerPtr = std::unique_ptr<Deserializer>;

                using Sender = std::function<DeserializerPtr (std::string const &, Serializer &)>;

                Proxy(IObjectManagerPtr manager, std::string const &serviceId, std::string const &interfaceId,
                        Sender && sender, StubCreator && stubCreator)
                    : m_manager{manager}
                    , m_instance{m_manager->CreateObject(serviceId, interfaceId)}
                    , m_sender{std::move(sender)}
                    , m_stubCreator{std::move(stubCreator)}
                {
                }

                Proxy(IObjectManagerPtr manager, std::string const &instance,
                        Sender && sender, StubCreator && stubCreator)
                    : m_manager{manager}
                    , m_instance{instance}
                    , m_sender{std::move(sender)}
                    , m_stubCreator{stubCreator}
                {
                }

                Proxy(std::string const &instance, Sender && sender, StubCreator && stubCreator)
                    : m_instance{instance}
                    , m_sender{std::move(sender)}
                    , m_stubCreator{stubCreator}
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
                        ObjectCleaner cleaner{m_manager};
                        Serializer serializer(true, requestId, m_instance, interface, method,
                                PrepareParam(std::forward<TParams>(params), cleaner) ... );
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

                bool QueryRemoteInterface(void **service, std::type_info const &typeInfo,
                        std::string const &serviceId, Service::IService **holder)
                {
                    return CreateProxy<0>(service, std::type_index{typeInfo}, serviceId, holder);
                }

            private:
                Common::UuidGenerator m_generator;
                IObjectManagerPtr m_manager;
                std::string m_instance;
                Sender m_sender;
                StubCreator m_stubCreator;

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
                    StubCreator stubCreator{m_stubCreator};

                    return Service::Make<ProxyType, InterfaceType>(m_manager, instanceId,
                            std::move(sender), std::move(stubCreator));
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
                bool CreateProxy(typename Registry::template Item<I>::Index::value_type, void **service,
                        std::type_index const &typeId, std::string const &serviceId, Service::IService **holder)
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
                        StubCreator stubCreator{m_stubCreator};
                        auto proxy = Service::Make<ProxyType, InterfaceType>(m_manager, instanceId,
                                std::move(sender), std::move(stubCreator));
                        *service = proxy.get();
                        (*holder = proxy->template Cast<Service::IService>().get())->AddRef();
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
                typename std::enable_if<I == Common::Detail::FakeHierarchyLength::value, bool>::type
                CreateProxy(void **, std::type_index const &, std::string const &, Service::IService **)
                {
                    return false;
                }

                template <std::size_t I>
                typename std::enable_if<I != Common::Detail::FakeHierarchyLength::value, bool>::type
                CreateProxy(void **service, std::type_index const &typeId, std::string const &serviceId, Service::IService **holder)
                {
                    if (CreateProxy<I>(std::size_t{}, service, typeId, serviceId, holder))
                        return true;
                    return CreateProxy<I + 1>(service, typeId, serviceId, holder);
                }

                // Specialization for all types that are not inherited from IService and not IService
                template <typename T>
                typename std::enable_if
                    <
                        !std::is_base_of<Service::IService, ExtractType<T>>::value &&
                        !std::is_same<Service::IService, ExtractType<T>>::value &&
                        !IsTServicePtr<ExtractType<T>>()
                        ,
                        T
                    >::type &&
                PrepareParam(T && param, ObjectCleaner &)
                {
                    return std::forward<T>(param);
                }

                // Specialization for pointers on interfaces based on IService
                template <typename T>
                typename std::enable_if
                    <
                        std::is_pointer<T>::value &&
                            (
                                std::is_base_of<Service::IService, ExtractType<T>>::value ||
                                std::is_same<Service::IService, ExtractType<T>>::value
                            )
                        ,
                        std::string
                    >::type
                PrepareParam(T && param, ObjectCleaner &cleaner)
                {
                    return PrepareParam(Service::TIntrusivePtr<ExtractType<T>>{param}, cleaner);
                }

                // Specialization for references on interfaces based on IService
                template <typename T>
                typename std::enable_if
                    <
                        std::is_reference<T>::value &&
                            (
                                std::is_base_of<Service::IService, ExtractType<T>>::value ||
                                std::is_same<Service::IService, ExtractType<T>>::value
                            )
                        ,
                        std::string
                    >::type
                PrepareParam(T && param, ObjectCleaner &cleaner)
                {
                    return PrepareParam(&param, cleaner);
                }

                // Specialization for smart pointers on interfaces based on IService
                template <typename T>
                typename std::enable_if
                    <
                        !std::is_pointer<T>::value &&
                        IsTServicePtr<ExtractType<T>>()
                        ,
                        std::string
                    >::type
                PrepareParam(T && param, ObjectCleaner &cleaner)
                {
                    if (!param)
                        return {};
                    using InterfaceType = typename ExtractType<T>::element_type;
                    using PSType = typename Registry::Registry<InterfaceType>::template Type<TSerializer>;
                    auto instanceId = m_stubCreator(std::forward<T>(param), PSType::InterfaceId);
                    cleaner.AppendId(instanceId);
                    return instanceId;
                }
            };

            template
                <
                    typename T,
                    bool = false ||
                        (
                            (   // Condition for pointers on interfaces based on IService
                                std::is_pointer<T>::value &&
                                    (
                                        std::is_base_of<Service::IService, ExtractType<T>>::value ||
                                        std::is_same<Service::IService, ExtractType<T>>::value
                                    )
                            )
                            ||
                            (   // Condition for references on interfaces based on IService
                                std::is_reference<T>::value &&
                                    (
                                        std::is_base_of<Service::IService, ExtractType<T>>::value ||
                                        std::is_same<Service::IService, ExtractType<T>>::value
                                    )
                            )
                            ||
                            (   // Condition for smart pointers on interfaces based on IService
                                !std::is_pointer<T>::value &&
                                IsTServicePtr<ExtractType<T>>()
                            )
                        )
                >
            struct InterfaceTypeToString
            {
                using Type = T;
            };

            template <typename T>
            struct InterfaceTypeToString<T, true>
            {
                using Type = std::string;
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

                using Sender = typename Proxy<TSerializer>::Sender;

                Stub(Service::IServicePtr instance, std::string const &instanceId,
                        Service::TIntrusivePtr<IObjectManager> manager,
                        StubCreator && stubCreator, Sender && sender)
                    : m_instance{instance}
                    , m_instanceId{instanceId}
                    , m_manager{manager}
                    , m_stubCreator(std::move(stubCreator))
                    , m_sender{std::move(sender)}
                {
                }

                Stub(Service::IServicePtr instance, std::string const &instanceId)
                    : m_instance{instance}
                    , m_instanceId{instanceId}
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

             private:
                Service::IServicePtr m_instance;
                std::string m_instanceId;
                Service::TIntrusivePtr<IObjectManager> m_manager;
                StubCreator m_stubCreator;
                Sender m_sender;
                using Services = std::list<Service::IServicePtr>;

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
                typename std::enable_if<I == Common::Detail::FakeHierarchyLength::value, Service::IServicePtr>::type
                QueryInterface(std::string const &, std::string const &)
                {
                    return {};
                }

                template <std::size_t I>
                typename std::enable_if<I != Common::Detail::FakeHierarchyLength::value, Service::IServicePtr>::type
                QueryInterface(std::string const &interfaceId, std::string const &serviceId)
                {
                    if (auto instance = QueryInterface<I>(std::size_t{}, interfaceId, serviceId))
                        return instance;
                    return QueryInterface<I + 1>(interfaceId, serviceId);
                }

            protected:
                virtual void InvokeMethod(std::string const &method, Deserializer &, Serializer &)
                {
                    throw ProxyStubException{"[Mif::Remote::Stub::InvokeMethod] Method \"" + method + "\" not found."};
                }

                virtual bool ContainInterfaceId(std::string const &id) const
                {
                    return false;
                }

                template <typename TResult, typename TInterface, typename ... TParams>
                void InvokeRealMethod(TResult (*method)(TInterface &, std::tuple<TParams ... > && ),
                                      Deserializer &deserializer, Serializer &serializer)
                {
                    auto inst = Service::Cast<TInterface>(m_instance);
                    auto tmpParams = deserializer.template GetParams<typename InterfaceTypeToString<TParams>::Type ... >();
                    Services services;
                    auto params = PrepareParams<TParams ... >(std::move(tmpParams), services,
                            static_cast<Common::MakeIndexSequence<sizeof ... (TParams)> const *>(nullptr));
                    FunctionWrap<TResult, TSerializer>::Call(
                            [&method, &inst, &params] ()
                            {
                                return method(*inst, std::move(params));
                            },
                            serializer,
                            m_stubCreator
                        );
                }

                template <typename ... TOutParams, typename ... TInParams, std::size_t ... Indexes>
                std::tuple<typename std::decay<TOutParams>::type ... >
                PrepareParams(std::tuple<TInParams ... > && input, Services &services,
                        Common::IndexSequence<Indexes ... > const *)
                {
                    return std::make_tuple(PrepareParam<TOutParams>(std::get<Indexes>(input), services) ... );
                }

                // Specialization for all types that are not inherited from IService and not IService
                template <typename T>
                typename std::enable_if
                    <
                        !std::is_base_of<Service::IService, ExtractType<T>>::value &&
                        !std::is_same<Service::IService, ExtractType<T>>::value &&
                        !IsTServicePtr<ExtractType<T>>(),
                        T
                    >::type &&
                PrepareParam(T && param, Services &)
                {
                    return std::forward<T>(param);
                }

                // Specialization for pointers on interfaces based on IService
                template <typename T>
                typename std::enable_if
                    <
                        std::is_pointer<T>::value &&
                            (
                                std::is_base_of<Service::IService, ExtractType<T>>::value ||
                                std::is_same<Service::IService, ExtractType<T>>::value
                            )
                        ,
                        T
                    >::type
                PrepareParam(std::string const &param, Services &services)
                {
                    return PrepareParam<Service::TServicePtr<ExtractType<T>>>(param, services).get();
                }

                /*
                // Specialization for references on interfaces based on IService
                template <typename T>
                typename std::enable_if
                    <
                        std::is_reference<T>::value &&
                            (
                                std::is_base_of<Service::IService, ExtractType<T>>::value ||
                                std::is_same<Service::IService, ExtractType<T>>::value
                            )
                        ,
                        ExtractType<T>
                    >::type &
                PrepareParam(std::string const &param, Services &services)
                {
                    (void)param;
                    (void)services;
                    throw std::runtime_error{"Not implemented."};
                }
                */

                // Specialization for smart pointers on interfaces based on IService
                template <typename T>
                typename std::enable_if
                    <
                        !std::is_pointer<T>::value &&
                        IsTServicePtr<ExtractType<T>>()
                        ,
                        T
                    >::type
                PrepareParam(std::string const &param, Services &services)
                {
                    // TODO: add implementation for reference on smart pointer
                    if (param.empty())
                        return {};
                    StubCreator stubCreator{m_stubCreator};
                    Sender sender{m_sender};
                    // TODO: need to clone instance id
                    std::string instanceId = param; //m_manager->CloneReference(param);
                    using InterfaceType = typename ExtractType<T>::element_type;
                    using PSType = typename Registry::Registry<InterfaceType>::template Type<TSerializer>;
                    using ProxyType = typename PSType::Proxy;;
                    auto instance = Service::Make<ProxyType>(instanceId, std::move(sender), std::move(stubCreator));
                    services.push_back(instance);
                    return instance->template Cast<InterfaceType>();
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


#endif  // !__MIF_REMOTE_DETAIL_PS_H__
