#include <iostream>
#include <tuple>
#include <cstdint>
#include <utility>

namespace Mif
{
    namespace Common
    {

        template <std::size_t ... Indexes>
        struct IndexSequence
        {
        };

        namespace Detail
        {

            template <typename>
            struct Method;

            template <typename TResult, typename TClass, typename ... TParams>
            struct Method<TResult (TClass::*)(TParams ...)>
            {
                using ClassType = TClass;
                using ResultType = TResult;
                using ParamTypeList = std::tuple<TParams ... >;
                enum { IsConst = 0 };
            };

            template <typename TResult, typename TClass, typename ... TParams>
            struct Method<TResult (TClass::*)(TParams ...) const>
            {
                using ClassType = TClass;
                using ResultType = TResult;
                using ParamTypeList = std::tuple<TParams ... >;
                enum { IsConst = 1 };
            };

            template <std::size_t N>
            struct Hierarchy
                : public Hierarchy<N - 1>
            {
            };

            template <>
            struct Hierarchy<0>
            {
            };

            template <std::size_t N>
            using MakeHierarchy = Hierarchy<N>;


            template <std::size_t N, std::size_t ... Indexes>
            struct IndexSequenceGenerator
                : IndexSequenceGenerator<N - 1, N - 1, Indexes ... >
            {
            };

            template <std::size_t ... Indexes>
            struct IndexSequenceGenerator<0u, Indexes ... >
            {
                using Type = IndexSequence<Indexes ... >;
            };

        }   // namespace Detail

        template <std::size_t N>
        using MakeIndexSequence = typename Detail::IndexSequenceGenerator<N>::Type;

    }   // namespace Common
}   // namespace Mif

#define MIF_REMOTE_PS_BEGIN(interface_) \
    template <typename TProxyBase> \
    class interface_ ## _PS \
    { \
    private: \
        using InterfaceType = interface_; \
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
            TResult _Mif_Remote_Call_Method(std::string const &method, TParams && ... params) const \
            { \
                return m_proxy.template RemoteCall<TResult>(method, std::forward<TParams>(params) ... ); \
            } \
        }; \
        using FakeHierarchy = ::Mif::Common::Detail::MakeHierarchy<100>; \
        static char (&GetNextCounter(void *))[1]; \
        static ProxyBase* GetBase(::Mif::Common::Detail::Hierarchy<1>);

#define MIF_REMOTE_PS_END() \
        using MethodProxies = typename std::remove_pointer<decltype(GetBase(FakeHierarchy{}))>::type; \
    public: \
        class Proxy \
            : public MethodProxies \
        { \
        public: \
            using MethodProxies::MethodProxies; \
        }; \
    };

#define MIF_REMOTE_DETAIL_METHOD_IMPL(method_, const_) \
    template <std::size_t ... Indexes> \
    class method_ ## _Mif_Remote_Proxy ## _ ## const_ \
        : public method_ ## _Base_Type \
    { \
    private: \
        using ResultType = typename method_ ## _Info ::ResultType; \
    public: \
        using method_ ## _Base_Type :: method_ ## _Base_Type ; \
        virtual ResultType method_ \
                (typename std::tuple_element<Indexes, typename method_ ## _Info ::ParamTypeList>::type ... params) \
            const_ override \
        { \
            static_assert(!std::is_pointer<ResultType>::value && !std::is_reference<ResultType>::value, \
                "Method \"" #method_ "\" must not return pointer or reference. Only value."); \
            return this->template _Mif_Remote_Call_Method<ResultType> \
                ( \
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
        method_ ## _Method_Type_Calc_ ## const_ (::Mif::Common::IndexSequence<Indexes ... >);

#define MIF_REMOTE_METHOD(method_) \
    using method_ ## _Info = ::Mif::Common::Detail::Method<decltype(&InterfaceType :: method_)>; \
    enum { method_ ## _Index = sizeof(GetNextCounter(static_cast<FakeHierarchy *>(nullptr))) }; \
    using method_ ## _Base_Type = typename std::remove_pointer<decltype(GetBase(::Mif::Common::Detail::Hierarchy<method_ ## _Index>{}))>::type; \
    MIF_REMOTE_DETAIL_METHOD_IMPL(method_, ) \
    MIF_REMOTE_DETAIL_METHOD_IMPL(method_, const) \
    using method_ ## _Type = typename std::conditional \
        < \
            method_ ## _Info :: IsConst, \
            decltype(method_ ## _Method_Type_Calc_const( \
                ::Mif::Common::MakeIndexSequence<std::tuple_size<typename method_ ## _Info ::ParamTypeList>::value>{})), \
            decltype(method_ ## _Method_Type_Calc_( \
                ::Mif::Common::MakeIndexSequence<std::tuple_size<typename method_ ## _Info ::ParamTypeList>::value>{})) \
        >::type; \
    static method_ ## _Type* GetBase(::Mif::Common::Detail::Hierarchy<method_ ## _Index + 1>); \
    static char (&GetNextCounter(::Mif::Common::Detail::Hierarchy<method_ ## _Index> *))[method_ ## _Index + 1];

namespace Mif
{
    namespace Remote
    {

        template <typename TSerializer, typename TTransport>
        class Proxy
        {
        public:
            virtual ~Proxy() = default;
            Proxy()
            {
            }

        public:
            template <typename TResult, typename ... TParams>
            TResult RemoteCall(std::string const &method, TParams && ... params)
            {
                throw std::runtime_error{method + " not implemented."}; \
            }
        };

    }   //  namespace Remote
}   // namespace Mif

struct ITest
{
    virtual ~ITest() = default;
    virtual void Print() = 0;
    virtual void SetVersion(int major, int minor) = 0;
    virtual void SetName(std::string const &name) = 0;
    virtual std::string  GetName() = 0;
    virtual int GetMajor() const = 0;
    virtual int GetMinor() const = 0;
};

MIF_REMOTE_PS_BEGIN(ITest)
    MIF_REMOTE_METHOD(Print)
    MIF_REMOTE_METHOD(SetVersion)
    MIF_REMOTE_METHOD(SetName)
    MIF_REMOTE_METHOD(GetName)
    MIF_REMOTE_METHOD(GetMajor)
    MIF_REMOTE_METHOD(GetMinor)
MIF_REMOTE_PS_END()

int main()
{
	try
	{
        ITest_PS<Mif::Remote::Proxy<struct Serializer, struct Transport>>::Proxy proxy;
        ITest &rps = proxy;
        rps.Print();
        rps.GetName();
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}
