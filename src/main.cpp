#include <iostream>
#include <functional>
#include <memory>
#include <vector>

/*
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include "mif/remote/ps.h"
#include "mif/remote/proxy_stub.h"
#include "mif/remote/serialization/serialization.h"
#include "mif/remote/serialization/boost/serialization.h"

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

class TestTransport;
TestTransport *p_transport = nullptr;

class TestTransport
{
public:
    virtual ~TestTransport() = default;
    TestTransport()
    {
    }

    using Buffer = std::vector<char>;
    using DataHandler = std::function<Buffer (Buffer &&)>;

    DataHandler m_handler;

    void SetHandler(DataHandler && handler)
    {
        p_transport = this;
        m_handler = std::move(handler);
    }

    Buffer Send(Buffer && buffer);

    Buffer Call(Buffer && buffer)
    {
        return m_handler(std::move(buffer));
    }
};

TestTransport::Buffer TestTransport::Send(Buffer && buffer)
{
    return p_transport->Call(std::move(buffer));
}

int main()
{
	try
	{
        using BoostSerializer = Mif::Remote::Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
        using BoostDeserializer = Mif::Remote::Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
        using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<BoostSerializer, BoostDeserializer>;
        using ProxyStub = ITest_PS<Mif::Remote::Proxy<SerializerTraits, TestTransport>, Mif::Remote::Stub<SerializerTraits, TestTransport>>;

        TestTransport proxyTransport;
        TestTransport stubTransport;
        ProxyStub::Stub stub("100500", std::make_shared<Test>(), std::move(stubTransport));
        stub.Init();
        ProxyStub::Proxy proxy("100500", std::move(proxyTransport));

        ITest &rps = proxy;
        std::cout << "Old name: " << rps.GetName() << std::endl;
        std::cout << "Old major: " << rps.GetMajor() << std::endl;
        std::cout << "Old minor: " << rps.GetMinor() << std::endl;
        std::cout << "--------------------------- RPC ---------------------------" << std::endl;
        rps.SetName("New name");
        rps.SetVersion(100, 500);
        std::cout << "--------------------------- RPC ---------------------------" << std::endl;
        std::cout << "New name: " << rps.GetName() << std::endl;
        std::cout << "New major: " << rps.GetMajor() << std::endl;
        std::cout << "New minor: " << rps.GetMinor() << std::endl;
    }
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}
*/

namespace Mif
{
    namespace Remote
    {
    }   // namespace Remote
}   // namespace Mif

namespace Mif
{
    namespace Net
    {

        using Buffer = std::vector<char>;

        struct IControl
        {
            virtual ~IControl() = default;
            virtual void CloseMe() = 0;
        };

        using IControlPtr = std::shared_ptr<IControl>;

        struct IPublisher
        {
            virtual ~IPublisher() = default;
            virtual void Publish(Buffer buffer) = 0;
        };

        using IPublisherPtr = std::shared_ptr<IPublisher>;

        struct ISubscriber
        {
            virtual ~ISubscriber() = default;
            virtual void OnData(Buffer buffer) = 0;
        };

        using ISubscriberPtr = std::shared_ptr<ISubscriber>;

        class Client
            : public std::enable_shared_from_this<Client>
            , public ISubscriber
        {
        public:
            virtual ~Client() = default;

            Client(IControlPtr control, IPublisherPtr publisher)
            {
                (void)control;
                (void)publisher;
            }

            // ISubscriber
            virtual void OnData(Buffer buffer) override final
            {
                throw std::runtime_error{"OnData not implemented."};
            }
        };

    }   // namespace Net
}   // namespace Mif

int main()
{
    try
    {
        Mif::Net::IControlPtr ctrl;
        Mif::Net::IPublisherPtr ptr;
        std::make_shared<Mif::Net::Client>(ctrl, ptr);
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
