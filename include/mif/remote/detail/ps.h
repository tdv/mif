//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2020 tdv
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
#include "mif/common/unused.h"
#include "mif/remote/detail/iobject_manager.h"
#include "mif/remote/detail/registry.h"
#include "mif/remote/detail/type_traits.h"
#include "mif/service/iservice.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Remote
    {
        namespace Detail
        {
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
                static typename std::enable_if<Traits::IsNotInterfaceValue<T>(), void>::type
                PackResult(T && res, Serializer &serializer, TStubCreator &)
                {
                    serializer.PutParams(std::forward<T>(res));
                }

                template <typename T, typename TStubCreator>
                static typename std::enable_if<Traits::IsTServicePtr<T>(), void>::type
                PackResult(T res, Serializer &serializer, TStubCreator &stubCreator)
                {
                    using PSType = typename Registry::Registry<typename T::element_type>::template Type<TSerializer>;
                    auto instanceId = stubCreator(res, std::string{PSType::InterfaceId});
                    serializer.PutParams(std::move(instanceId));
                }

                template <typename T, typename TStubCreator>
                static typename std::enable_if<Traits::IsPtrOrRef<T>(), void>::type
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
                    Common::Unused(id);
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

                Proxy(IObjectManagerPtr manager, Service::ServiceId serviceId, std::string const &interfaceId,
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
                    return Registry::Visitor::Accept<CreateProxyVisitor>(m_manager, m_instance,
                            static_cast<Sender const &>(m_sender), static_cast<StubCreator const &>(m_stubCreator),
                            service, std::type_index{typeInfo}, serviceId, holder);
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
                        Traits::IsNotInterfaceValue<TResult>(),
                        TResult
                    >::type
                ExtractResult(Deserializer &deserializer)
                {
                    return std::get<0>(deserializer.template GetParams<TResult>());
                }

                template <typename TResult>
                typename std::enable_if<Traits::IsTServicePtr<TResult>(), TResult>::type
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
                typename std::enable_if<Traits::IsPtrOrRef<TResult>(), TResult>::type
                ExtractResult(Deserializer &)
                {
                    static_assert(!std::is_pointer<TResult>::value && !std::is_reference<TResult>::value,
                        "You can't return a pointer or a reference from some interface method. Only value.");

                    throw std::exception{};
                }

                template <typename TResult>
                typename std::enable_if<std::is_same<TResult, void>::value, TResult>::type
                ExtractResult(Deserializer &de)
                {
                    Common::Unused(de);
                }

                struct CreateProxyVisitor
                {
                    using Serializer = TSerializer;
                    using Result = bool;

                    template <typename T>
                    static Result Visit(IObjectManagerPtr manager, std::string const &instance,
                            Sender const &sender, StubCreator const &stubCreator,
                            void **service, std::type_index const &typeId,
                            std::string const &serviceId, Service::IService **holder)
                     {
                         using InterfaceType = typename T::InterfaceType;
                         using ProxyType = typename T::Proxy;
                         if (std::type_index{typeid(InterfaceType)} == typeId)
                         {
                             auto const instanceId = manager->QueryInterface(instance, T::InterfaceId, serviceId);
                             if (instanceId.empty())
                                 return false;
                             Sender newSender{sender};
                             StubCreator newStubCreator{stubCreator};
                             auto proxy = Service::Make<ProxyType, InterfaceType>(manager, instanceId,
                                     std::move(newSender), std::move(newStubCreator));
                             *service = proxy.get();
                             (*holder = proxy->template Cast<Service::IService>().get())->AddRef();
                             return true;
                         }

                         return false;
                     }
                };

                // Specialization for all types that are not inherited from IService and not IService
                template <typename T>
                typename std::enable_if<Traits::IsNotInterface<T>(), T>::type &&
                PrepareParam(T && param, ObjectCleaner &)
                {
                    return std::forward<T>(param);
                }

                // Specialization for pointers on interfaces based on IService
                template <typename T>
                typename std::enable_if<Traits::IsInterfaceRawPtr<T>(), std::string>::type
                PrepareParam(T && param, ObjectCleaner &cleaner)
                {
                    return PrepareParam(Service::TIntrusivePtr<Traits::ExtractType<T>>{param}, cleaner);
                }

                // Specialization for references on interfaces based on IService
                template <typename T>
                typename std::enable_if<Traits::IsInterfaceRef<T>(), std::string>::type
                PrepareParam(T && param, ObjectCleaner &cleaner)
                {
                    return PrepareParam(&param, cleaner);
                }

                // Specialization for smart pointers on interfaces based on IService
                template <typename T>
                typename std::enable_if<Traits::IsInterfaceSmartPtr<T>(), std::string>::type
                PrepareParam(T && param, ObjectCleaner &cleaner)
                {
                    if (!param)
                        return {};
                    using InterfaceType = typename Traits::ExtractType<T>::element_type;
                    using PSType = typename Registry::Registry<InterfaceType>::template Type<TSerializer>;
                    auto instanceId = m_stubCreator(std::forward<T>(param), PSType::InterfaceId);
                    cleaner.AppendId(instanceId);
                    return instanceId;
                }
            };

            template
                <
                    typename T,
                    bool = Traits::IsInterfaceRawPtr<T>() || Traits::IsInterfaceRef<T>() ||
                            Traits::IsInterfaceSmartPtr<T>()
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
                virtual Service::IServicePtr GetInstance() = 0;
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
                    return Registry::Visitor::Accept<QueryInterfaceVisitor>(m_instance, interfaceId, serviceId);
                }

                virtual Service::IServicePtr GetInstance() override final
                {
                    return m_instance;
                }

             private:
                Service::IServicePtr m_instance;
                std::string m_instanceId;
                Service::TIntrusivePtr<IObjectManager> m_manager;
                StubCreator m_stubCreator;
                Sender m_sender;
                using Services = std::list<Service::IServicePtr>;

                struct QueryInterfaceVisitor
                {
                    using Serializer = TSerializer;
                    using Result = Service::IServicePtr;

                    template <typename T>
                    static Result Visit(Service::IServicePtr instance, std::string const &interfaceId,
                            std::string const &serviceId)
                    {
                        using InterfaceType = typename T::InterfaceType;
                        if (interfaceId == T::InterfaceId)
                        {
                            auto newInstance = Service::Query<InterfaceType>(instance, serviceId);
                            if (!newInstance)
                                return {};

                            return newInstance->template Cast<Service::IService>();
                        }

                        return {};
                    }
                };

            protected:
                virtual void InvokeMethod(std::string const &method, Deserializer &, Serializer &)
                {
                    throw ProxyStubException{"[Mif::Remote::Stub::InvokeMethod] Method \"" + method + "\" not found."};
                }

                virtual bool ContainInterfaceId(std::string const &id) const
                {
                    Common::Unused(id);
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
                typename std::enable_if<Traits::IsNotInterface<T>(), T>::type
                PrepareParam(T param, Services &)
                {
                    return param;
                }

                // Specialization for pointers on interfaces based on IService
                template <typename T>
                typename std::enable_if<Traits::IsInterfaceRawPtr<T>(), T>::type
                PrepareParam(std::string const &param, Services &services)
                {
                    return PrepareParam<Service::TServicePtr<Traits::ExtractType<T>>>(param, services).get();
                }

                /*
                // Specialization for references on interfaces based on IService
                template <typename T>
                typename std::enable_if<Traits::IsInterfaceRef<T>(), Traits::ExtractType<T>>::type &
                PrepareParam(std::string const &param, Services &services)
                {
                    Common::Unused(param);
                    Common::Unused(services);
                    throw std::runtime_error{"Not implemented."};
                }
                */

                // Specialization for smart pointers on interfaces based on IService
                template <typename T>
                typename std::enable_if<Traits::IsInterfaceSmartPtr<T>(), T>::type
                PrepareParam(std::string const &param, Services &services)
                {
                    // TODO: add implementation for reference on smart pointer
                    if (param.empty())
                        return {};
                    StubCreator stubCreator{m_stubCreator};
                    Sender sender{m_sender};
                    using InterfaceType = typename Traits::ExtractType<T>::element_type;
                    using PSType = typename Registry::Registry<InterfaceType>::template Type<TSerializer>;
                    auto const instanceId = m_manager->CloneReference(param, PSType::InterfaceId);
                    using ProxyType = typename PSType::Proxy;
                    auto instance = Service::Make<ProxyType>(instanceId, std::move(sender), std::move(stubCreator));
                    services.push_back(instance);
                    return instance->template Cast<InterfaceType>();
                }
            };

        }  // namespace Detail
    }   //  namespace Remote
}   // namespace Mif


#endif  // !__MIF_REMOTE_DETAIL_PS_H__
