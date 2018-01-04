//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_PS_H__
#define __MIF_REMOTE_PS_H__

// STD
#include <cstdint>
#include <string>
#include <tuple>
#include <type_traits>

// MIF
#include "mif/common/index_sequence.h"
#include "mif/common/detail/method.h"
#include "mif/remote/detail/ps_base.h"

#define MIF_REMOTE_PS_BEGIN(interface_) \
    template <typename TSerializer> \
    class interface_ ## _PS \
    { \
    public: \
        using ThisType = interface_ ## _PS <TSerializer>; \
        using InterfaceType = interface_; \
        static constexpr auto InterfaceId = #interface_; \
    private: \
        template <typename TBase> \
        class ProxyBase \
            : public TBase \
        { \
        protected: \
            using TBase::TBase; \
        }; \
        static char (&GetNextCounter(void *))[1]; \
        template <typename TBase> \
        static ProxyBase<TBase>* GetProxyBase(::Mif::Common::Detail::Hierarchy<1>); \
        template <typename TBase> \
        class StubBase \
            : public TBase \
        { \
        protected: \
            using TBase::TBase; \
        }; \
        template <typename TBase> \
        static StubBase<TBase> GetStubBase(::Mif::Common::Detail::Hierarchy<1>);

#define MIF_REMOTE_PS_END() \
        template <typename TBase> \
        using MethodProxies = typename std::remove_pointer<decltype(ThisType::GetProxyBase<TBase>( \
                ::Mif::Common::Detail::FakeHierarchy{}))>::type; \
        template <typename TBase> \
        using MethodStubs = decltype(ThisType::GetStubBase<TBase>(::Mif::Common::Detail::FakeHierarchy{})); \
    public: \
        template <typename TBase = ::Mif::Remote::Detail::InheritProxy<TSerializer, InterfaceType>> \
        using ProxyItem = MethodProxies<TBase>; \
        using Proxy = ProxyItem<>; \
        template <typename TBase = ::Mif::Remote::Detail::InheritStub<TSerializer, InterfaceType>> \
        class StubItem \
            : public MethodStubs<TBase> \
        { \
        protected: \
            using MethodStubs<TBase>::MethodStubs; \
            using Serializer = typename MethodStubs<TBase>::Serializer; \
            using Deserializer = typename MethodStubs<TBase>::Deserializer; \
            virtual bool ContainInterfaceId(std::string const &id) const \
            { \
                return InterfaceId == id || MethodStubs<TBase>::ContainInterfaceId(id); \
            } \
            virtual void InvokeMethod(std::string const &method, Deserializer &deserializer, Serializer &serializer) \
            { \
                MethodStubs<TBase>::InvokeMethod(method, deserializer, serializer); \
            } \
        }; \
        using Stub = StubItem<>; \
    };

#define MIF_REMOTE_DETAIL_PROXY_METHOD_IMPL(method_, const_) \
    template <typename TBase, std::size_t ... Indexes> \
    class method_ ## _Mif_Remote_Proxy ## _ ## const_ \
        : public method_ ## _Proxy_Base_Type<TBase> \
    { \
    private: \
        using ResultType = typename method_ ## _Info ::ResultType; \
    protected: \
        using method_ ## _Proxy_Base_Type <TBase> :: method_ ## _Proxy_Base_Type ; \
    private: \
        virtual ResultType method_ \
                (typename std::tuple_element<Indexes, typename method_ ## _Info ::ParamTypeList>::type ... params) \
            const_ override final \
        { \
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
    template <typename TBase, std::size_t ... Indexes> \
    static method_ ## _Mif_Remote_Proxy_ ## const_ <TBase, Indexes ... > \
        method_ ## _Method_Proxy_Type_Calc_ ## const_ (::Mif::Common::IndexSequence<Indexes ... >);

#define MIF_REMOTE_METHOD(method_) \
    using method_ ## _Info = ::Mif::Common::Detail::Method<decltype(&InterfaceType :: method_)>; \
    enum { method_ ## _Index = sizeof(GetNextCounter(static_cast<::Mif::Common::Detail::FakeHierarchy *>(nullptr))) }; \
    using method_ ## _IndexSequence = ::Mif::Common::MakeIndexSequence<std::tuple_size<typename method_ ## _Info ::ParamTypeList>::value>; \
    template <typename TBase> \
    using method_ ## _Proxy_Base_Type = typename std::remove_pointer<decltype(ThisType::GetProxyBase<TBase>(::Mif::Common::Detail::Hierarchy<method_ ## _Index>{}))>::type; \
    template <typename TBase> \
    using method_ ## _Stub_Base_Type = decltype(ThisType::GetStubBase<TBase>(::Mif::Common::Detail::Hierarchy<method_ ## _Index>{})); \
    MIF_REMOTE_DETAIL_PROXY_METHOD_IMPL(method_, ) \
    MIF_REMOTE_DETAIL_PROXY_METHOD_IMPL(method_, const) \
    template <typename TBase> \
    using method_ ## _Proxy_Type = typename std::conditional \
        < \
            method_ ## _Info :: IsConst, \
            decltype(ThisType :: method_ ## _Method_Proxy_Type_Calc_const<TBase>(method_ ## _IndexSequence{})), \
            decltype(ThisType :: method_ ## _Method_Proxy_Type_Calc_<TBase>(method_ ## _IndexSequence{})) \
        >::type; \
    template <typename TBase> \
    static method_ ## _Proxy_Type<TBase>* GetProxyBase(::Mif::Common::Detail::Hierarchy<method_ ## _Index + 1>); \
    template <typename TBase, std::size_t ... Indexes> \
    class method_ ## _Mif_Remote_Stub \
        : public method_ ## _Stub_Base_Type<TBase> \
    { \
    private: \
        using ResultType = typename method_ ## _Info ::ResultType; \
        using ParamTypeList = typename method_ ## _Info ::ParamTypeList; \
    protected: \
        using BaseType = method_ ## _Stub_Base_Type<TBase>; \
        using BaseType::BaseType; \
        using Serializer = typename BaseType::Serializer; \
        using Deserializer = typename BaseType::Deserializer; \
        static ResultType Invoke(InterfaceType &instance, ParamTypeList && params) \
        { \
            return instance. method_ (std::get<Indexes>(params) ... ); \
        } \
        virtual void InvokeMethod(std::string const &method, Deserializer &deserializer, Serializer &serializer) override \
        { \
            if (method != #method_) \
                BaseType::InvokeMethod(method, deserializer, serializer); \
            else \
                BaseType::InvokeRealMethod(& method_ ## _Mif_Remote_Stub :: Invoke, deserializer, serializer); \
        } \
    }; \
    template <typename TBase, std::size_t ... Indexes> \
    static method_ ## _Mif_Remote_Stub <TBase, Indexes ... > \
        method_ ## _Method_Stub_Type_Calc (::Mif::Common::IndexSequence<Indexes ... >); \
    template <typename TBase> \
    using method_ ## _Stub_Type = \
        decltype(ThisType :: method_ ## _Method_Stub_Type_Calc<TBase>(method_ ## _IndexSequence{})); \
    template <typename TBase> \
    static method_ ## _Stub_Type<TBase> GetStubBase(::Mif::Common::Detail::Hierarchy<method_ ## _Index + 1>); \
    static char (&GetNextCounter(::Mif::Common::Detail::Hierarchy<method_ ## _Index> *))[method_ ## _Index + 1];

#endif  // !__MIF_REMOTE_PS_H__
