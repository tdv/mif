//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_PS_H__
#define __MIF_REMOTE_PS_H__

// STD
#include <cstdint>

// MIF
#include "mif/common/index_sequence.h"
#include "mif/common/detail/hierarchy.h"
#include "mif/common/detail/method.h"
#include "mif/remote/detail/ps.h"

namespace Mif
{
    namespace Remote
    {
        namespace Detail
        {

            using FakeHierarchy = Common::Detail::MakeHierarchy<100>;

            inline constexpr FakeHierarchy const* GetFakeHierarchy()
            {
                return static_cast<FakeHierarchy const *>(nullptr);
            }

            namespace Registry

            {
                namespace Counter
                {
                    inline constexpr std::size_t GetLast(void const *)
                    {
                        return 0;
                    }

                }   // namespace Counter

                template <typename TInterface>
                struct Registry;

                template <std::size_t I>
                struct Item;

            }   // namespace Registry
        }   // namespace Detail
    }   // namespace Remote
}   // namespace Mif

#define MIF_REMOTE_PS_BEGIN(interface_) \
    template <typename TSerializer> \
    class interface_ ## _PS \
    { \
    public: \
        using InterfaceType = interface_; \
        static constexpr auto InterfaceId = #interface_; \
    private: \
        class ProxyBase \
            : public InterfaceType \
        { \
        public: \
            template <typename ... TParams> \
            ProxyBase(TParams && ... params) \
                : m_proxy(std::forward<TParams>(params) ... ) \
            { \
            } \
        private: \
            mutable ::Mif::Remote::Detail::Proxy<TSerializer> m_proxy; \
        protected: \
            template <typename TResult, typename ... TParams> \
            TResult _Mif_Remote_Call_Method(std::string const &interfaceId, std::string const &method, TParams && ... params) const \
            { \
                return m_proxy.template RemoteCall<TResult>(interfaceId, method, std::forward<TParams>(params) ... ); \
            } \
        }; \
        using FakeHierarchy = ::Mif::Remote::Detail::FakeHierarchy; \
        static char (&GetNextCounter(void *))[1]; \
        static ProxyBase* GetProxyBase(::Mif::Common::Detail::Hierarchy<1>); \
        class StubBase \
            : public ::Mif::Remote::Detail::Stub<TSerializer> \
        { \
        public: \
            using ::Mif::Remote::Detail::Stub<TSerializer>::Stub; \
        }; \
        static StubBase GetStubBase(::Mif::Common::Detail::Hierarchy<1>);

#define MIF_REMOTE_PS_END() \
        using MethodProxies = typename std::remove_pointer<decltype(GetProxyBase(FakeHierarchy{}))>::type; \
        using MethodStubs = decltype(GetStubBase(FakeHierarchy{})); \
    public: \
        class Proxy \
            : public MethodProxies \
        { \
        public: \
            using MethodProxies::MethodProxies; \
        }; \
        class Stub \
            : public MethodStubs \
        { \
        public: \
            using MethodStubs::MethodStubs; \
        private: \
            virtual char const* GetInterfaceId() const override final \
            { \
                return InterfaceId; \
            } \
            virtual void InvokeMethod(void *instance, std::string const &method, void *deserializer, void *serializer) override final \
            { \
                MethodStubs::InvokeMethod(instance, method, deserializer, serializer); \
            } \
        }; \
    };

#define MIF_REMOTE_DETAIL_PROXY_METHOD_IMPL(method_, const_) \
    template <std::size_t ... Indexes> \
    class method_ ## _Mif_Remote_Proxy ## _ ## const_ \
        : public method_ ## _Proxy_Base_Type \
    { \
    private: \
        using ResultType = typename method_ ## _Info ::ResultType; \
    public: \
        using method_ ## _Proxy_Base_Type :: method_ ## _Proxy_Base_Type ; \
        virtual ResultType method_ \
                (typename std::tuple_element<Indexes, typename method_ ## _Info ::ParamTypeList>::type ... params) \
            const_ override final \
        { \
            static_assert(!std::is_pointer<ResultType>::value && !std::is_reference<ResultType>::value, \
                "Method \"" #method_ "\" must not return pointer or reference. Only value."); \
            return this->template _Mif_Remote_Call_Method<ResultType> \
                ( \
                    InterfaceId, \
                    #method_, \
                    std::forward \
                    < \
                        typename std::tuple_element<Indexes, typename method_ ## _Info ::ParamTypeList>::type \
                    > \
                    (params) \
                ... ); \
        } \
    }; \
    template <std::size_t ... Indexes> \
    static method_ ## _Mif_Remote_Proxy_ ## const_ <Indexes ... > \
        method_ ## _Method_Proxy_Type_Calc_ ## const_ (::Mif::Common::IndexSequence<Indexes ... >);

#define MIF_REMOTE_METHOD(method_) \
    using method_ ## _Info = ::Mif::Common::Detail::Method<decltype(&InterfaceType :: method_)>; \
    enum { method_ ## _Index = sizeof(GetNextCounter(static_cast<FakeHierarchy *>(nullptr))) }; \
    using method_ ## _IndexSequence = ::Mif::Common::MakeIndexSequence<std::tuple_size<typename method_ ## _Info ::ParamTypeList>::value>; \
    using method_ ## _Proxy_Base_Type = typename std::remove_pointer<decltype(GetProxyBase(::Mif::Common::Detail::Hierarchy<method_ ## _Index>{}))>::type; \
    using method_ ## _Stub_Base_Type = decltype(GetStubBase(::Mif::Common::Detail::Hierarchy<method_ ## _Index>{})); \
    MIF_REMOTE_DETAIL_PROXY_METHOD_IMPL(method_, ) \
    MIF_REMOTE_DETAIL_PROXY_METHOD_IMPL(method_, const) \
    using method_ ## _Proxy_Type = typename std::conditional \
        < \
            method_ ## _Info :: IsConst, \
            decltype(method_ ## _Method_Proxy_Type_Calc_const(method_ ## _IndexSequence{})), \
            decltype(method_ ## _Method_Proxy_Type_Calc_(method_ ## _IndexSequence{})) \
        >::type; \
    static method_ ## _Proxy_Type* GetProxyBase(::Mif::Common::Detail::Hierarchy<method_ ## _Index + 1>); \
    template <std::size_t ... Indexes> \
    class method_ ## _Mif_Remote_Stub \
        : public method_ ## _Stub_Base_Type \
    { \
    private: \
        using BaseType = method_ ## _Stub_Base_Type; \
        using ResultType = typename method_ ## _Info ::ResultType; \
        using ParamTypeList = typename method_ ## _Info ::ParamTypeList; \
    protected: \
        using BaseType::BaseType; \
        static ResultType Invoke(InterfaceType &instance, ParamTypeList && params) \
        { \
            static_assert(!std::is_pointer<ResultType>::value && !std::is_reference<ResultType>::value, \
                "Method \"" #method_ "\" must not return pointer or reference. Only value."); \
            return instance. method_ (std::get<Indexes>(params) ... ); \
        } \
        virtual void InvokeMethod(void *instance, std::string const &method, void *deserializer, void *serializer) override \
        { \
            if (method != #method_) \
                BaseType::InvokeMethod(instance, method, deserializer, serializer); \
            else \
                BaseType::InvokeRealMethod(& method_ ## _Mif_Remote_Stub :: Invoke, instance, deserializer, serializer); \
        } \
    }; \
    template <std::size_t ... Indexes> \
    static method_ ## _Mif_Remote_Stub <Indexes ... > \
        method_ ## _Method_Stub_Type_Calc (::Mif::Common::IndexSequence<Indexes ... >); \
    using method_ ## _Stub_Type = \
        decltype(method_ ## _Method_Stub_Type_Calc(method_ ## _IndexSequence{})); \
    static method_ ## _Stub_Type GetStubBase(::Mif::Common::Detail::Hierarchy<method_ ## _Index + 1>); \
    static char (&GetNextCounter(::Mif::Common::Detail::Hierarchy<method_ ## _Index> *))[method_ ## _Index + 1];

#define MIF_REMOTE_REGISTER_PS(interface_) \
    namespace Mif \
    { \
        namespace Remote \
        { \
            namespace Detail \
            { \
                namespace Registry \
                { \
                    template <> \
                    struct Registry< :: interface_> \
                    { \
                        static constexpr auto Id = Counter::GetLast(GetFakeHierarchy()) + 1; \
                        template <typename TSerializer> \
                        using Type = :: interface_ ## _PS <TSerializer>; \
                    }; \
                    template <> \
                    struct Item<Registry< :: interface_>::Id> \
                    { \
                        using Type = Registry< :: interface_>; \
                    }; \
                    namespace Counter \
                    { \
                        inline constexpr std::size_t GetLast(Common::Detail::MakeHierarchy \
                            < \
                                Registry< :: interface_>::Id \
                            > const *) \
                        { \
                            return Registry< :: interface_> ::Id; \
                        } \
                    } \
                } \
            } \
        } \
    } \


#endif  // !__MIF_REMOTE_PS_H__
