#include <iostream>
#include <tuple>
#include <cstdint>
#include <utility>
#include <sstream>
#include <vector>

#include <boost/iostreams/stream.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/serialization/string.hpp>


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

namespace Mif
{
    namespace Remote
    {

        template <typename TSerializer, typename TTransport>
        class Proxy
        {
        public:
            Proxy(std::string const &instance, TTransport transport)
                : m_instance(instance)
                , m_transport(std::move(transport))
            {
            }

            virtual ~Proxy() = default;

        public:
            template <typename TResult, typename ... TParams>
            TResult RemoteCall(std::string const &interface, std::string const &method, TParams && ... params)
            {
                try
                {
                    using Serializer = typename TSerializer::Serializer;
                    Serializer serializer(m_instance, interface, method, std::forward<TParams>(params) ... );
                    auto response = m_transport.Send(std::move(serializer.GetBuffer()));
                    throw std::runtime_error{method + " not implemented."};
                }
                catch (std::exception const &e)
                {
                    throw std::runtime_error{"[Mif::Remote::Proxy::RemoteCall] Failed to call remote method \"" +
                        interface + "::" + method + "\" for instance with id \"" + m_instance + "\". Error: " +
                        std::string{e.what()}};
                }
            }

        private:
            std::string m_instance;
            TTransport m_transport;
        };

    }   //  namespace Remote
}   // namespace Mif


namespace Mif
{
    namespace Remote
    {
        namespace Boost
        {
            namespace Serializer
            {
                namespace Detail
                {

                    struct Tag
                    {
                        static constexpr auto Type = "type";
                        static constexpr auto Request = "request";
                        static constexpr auto Response = "response";
                        static constexpr auto Instsnce = "instance";
                        static constexpr auto Interface = "interface";
                        static constexpr auto Method = "method";
                        static constexpr auto Param = "prm";
                    };

                }   // namespace Detail

                template <typename TArchive>
                class Serializer final
                {
                public:
                    using Buffer = std::vector<char>;

                    template <typename ... TParams>
                    Serializer(std::string const &instanceId, std::string const &interfaceId,
                        std::string const &methodId, TParams && ... params)
                    {
                        boost::iostreams::filtering_ostream stream(boost::iostreams::back_inserter(m_result));
                        TArchive archive(stream);
                        std::string type = Detail::Tag::Request;
                        archive << boost::serialization::make_nvp(Detail::Tag::Type, type);
                        archive << boost::serialization::make_nvp(Detail::Tag::Instsnce, instanceId);
                        archive << boost::serialization::make_nvp(Detail::Tag::Interface, interfaceId);
                        archive << boost::serialization::make_nvp(Detail::Tag::Method, methodId);
                        PutParams(archive, 1, std::forward<TParams>(params) ... );
                    }

                    Buffer  GetBuffer()
                    {
                        return std::move(m_result);
                    }

                private:
                    Buffer m_result;

                    template <typename TParam, typename ... TParams>
                    void PutParams(TArchive &archive, std::size_t index, TParam && param, TParams && ... params) const
                    {
                        archive << boost::serialization::make_nvp((Detail::Tag::Param + std::to_string(index)).c_str(), param);
                        PutParams(archive, index + 1, std::forward<TParams>(params) ... );
                    }

                    void PutParams(TArchive &, std::size_t) const
                    {
                    }
                };

                template <typename TArchive>
                class Deserializer final
                {
                public:
                    Deserializer()
                    {
                    }

                private:
                };

            }   // namespace Serializer
        }   // namespace Boost
    }   //  namespace Remote
}   // namespace Mif

namespace Mif
{
    namespace Remote
    {

        template <typename TSerializer, typename TDeserializer>
        class SerializerTraits final
        {
        public:
            using Serializer = TSerializer;
            using Deserivalizer = TDeserializer;
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

class Test final
    : public ITest
{
private:
    std::string m_name{"TestDefault"};
    int m_major{0};
    int m_minor{0};

    virtual void Print() override
    {
        std::cout << "[Test::Print] Name\"" << m_name << "\" Major " << m_major << " Minor " << m_minor << std::endl;
    }
    virtual void SetVersion(int major, int minor) override
    {
        std::cout << "[Test::SetVersion] New major " << major << " New minor " << minor << std::endl;
        m_major = major;
        m_minor = minor;
    }
    virtual void SetName(std::string const &name) override
    {
        std::cout << "[Test::SetName] New name \"" << name << "\"" << std::endl;
        m_name = name;
    }
    virtual std::string  GetName() override
    {
        std::cout << "[Test::GetName] Name \"" << m_name << "\"" << std::endl;
        return m_name;
    }
    virtual int GetMajor() const override
    {
        std::cout << "[Test::GetMajor] Major " << m_major << std::endl;
        return m_major;
    }
    virtual int GetMinor() const override
    {
        std::cout << "[Test::GetMinor] Minor " << m_minor << std::endl;
        return m_minor;
    }
};

class TestTransport final
{
public:
    using Buffer = std::vector<char>;
    Buffer Send(Buffer buffer)
    {
        throw std::runtime_error{"TestTransport::Send not implemented."};
    }
};

int main()
{
	try
	{
        using BoostSerializer = Mif::Remote::Boost::Serializer::Serializer<boost::archive::xml_oarchive>;
        using BoostDeserializer = Mif::Remote::Boost::Serializer::Deserializer<boost::archive::xml_iarchive>;
        //using BoostDeserializer = Mif::Remote::Boost::Serivalizer<boost::archive::xml_iarchive;
        using S = Mif::Remote::SerializerTraits<BoostSerializer, BoostDeserializer>;

        TestTransport transport;
        ITest_PS<Mif::Remote::Proxy<S, TestTransport>>::Proxy proxy("100500", std::move(transport));
        ITest_PS<Mif::Remote::Proxy<S, TestTransport>>::Stub stub;
        (void)stub;
        ITest &rps = proxy;
        rps.SetName("New name");
        rps.Print();
        rps.GetName();
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}
