#ifndef __MIF_REMOTE_PS_H__
#define __MIF_REMOTE_PS_H__

// STD
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <tuple>
#include <utility>

// MIF
#include "mif/common/index_sequence.h"
#include "mif/common/detail/hierarchy.h"
#include "mif/common/detail/method.h"

#define MIF_REMOTE_PS_BEGIN(interface_) \
    template <typename TProxyBase> \
    class interface_ ## _PS \
    { \
    private: \
        using InterfaceType = interface_; \
        static constexpr auto InterfaceName = #interface_; \
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
            mutable TProxyBase m_proxy; \
        protected: \
            template <typename TResult, typename ... TParams> \
            TResult _Mif_Remote_Call_Method(std::string const &interfaceId, std::string const &method, TParams && ... params) const \
            { \
                return m_proxy.template RemoteCall<TResult>(interfaceId, method, std::forward<TParams>(params) ... ); \
            } \
        }; \
        using FakeHierarchy = ::Mif::Common::Detail::MakeHierarchy<100>; \
        static char (&GetNextCounter(void *))[1]; \
        static ProxyBase* GetProxyBase(::Mif::Common::Detail::Hierarchy<1>); \
        class StubBase \
        { \
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
            const_ override \
        { \
            static_assert(!std::is_pointer<ResultType>::value && !std::is_reference<ResultType>::value, \
                "Method \"" #method_ "\" must not return pointer or reference. Only value."); \
            return this->template _Mif_Remote_Call_Method<ResultType> \
                ( \
                    InterfaceName, \
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
    public: \
        template <typename TReslt, typename TParamsTuple> \
        static TReslt Invoker(InterfaceType &instance, TParamsTuple && params) \
        { \
            instance. method_ (std::get<Indexes>(params) ... ); \
            throw std::runtime_error{"Stub for " #method_  " not implemented."}; \
        } \
    }; \
    template <std::size_t ... Indexes> \
    static method_ ## _Mif_Remote_Stub <Indexes ... > \
        method_ ## _Method_Stub_Type_Calc (::Mif::Common::IndexSequence<Indexes ... >); \
    using method_ ## _Stub_Type = \
        decltype(method_ ## _Method_Stub_Type_Calc(method_ ## _IndexSequence{})); \
    static method_ ## _Stub_Type GetStubBase(::Mif::Common::Detail::Hierarchy<method_ ## _Index + 1>); \
    static char (&GetNextCounter(::Mif::Common::Detail::Hierarchy<method_ ## _Index> *))[method_ ## _Index + 1];

#endif  // !__MIF_REMOTE_PS_H__
