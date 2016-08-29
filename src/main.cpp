#include <iostream>
#include <functional>
#include <memory>
#include <vector>

#include <thread>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/thread.hpp>


#include "mif/net/client.h"
#include "mif/net/isubscriber_factory.h"

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
        class ClientFactory final
            : public ISubscriberFactory
        {
        public:
        private:
            // ISubscriberFactory
            virtual std::shared_ptr<ISubscriber> Create(std::weak_ptr<IControl> control,
                std::weak_ptr<IPublisher> publisher) override
            {
                return std::make_shared<Client>(control, publisher);
            }
        };

    }   // namespace Net
}   // namespace Mif

#include "mif/net/tcp_server.h"
#include "mif/net/tcp_clients.h"


class ClientFactory final
    : public Mif::Net::ISubscriberFactory
{
public:
    /*void PostData()
    {
        for (auto client : m_clients)
        {
            if (auto publisher = client->m_publisher.lock())
            {
                std::string s = "GET / HTTP/1.1\r\n\r\n";
                boost::shared_array<char> b(new char [s.length() + 1]);
                std::strcpy(b.get(), s.c_str());
                Mif::Net::Buffer buffer = std::make_pair(s.length(), b);
                publisher->Publish(buffer);
            }
        }
    }*/

private:
    std::list<std::shared_ptr<Mif::Net::Client>> m_clients;
    // ISubscriberFactory
    virtual std::shared_ptr<Mif::Net::ISubscriber> Create(std::weak_ptr<Mif::Net::IControl> control,
        std::weak_ptr<Mif::Net::IPublisher> publisher) override
    {
        auto client = std::make_shared<Mif::Net::Client>(control, publisher);
        m_clients.push_back(client);
        return client;
    }
};

int main()
{
    try
    {
        auto server = std::make_shared<Mif::Net::TCPServer>(
            "localhost", "5555", 4, std::make_shared<Mif::Net::ClientFactory>());
        auto clientFactgory = std::make_shared<ClientFactory>();
        Mif::Net::TCPClients clients(4, clientFactgory);
        /*for (int i = 0 ; i < 10 ; ++i)
            clients.RunClient("localhost", "5555");
        clientFactgory->PostData();*/
        std::cin.get();
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
