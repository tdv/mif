#include <iostream>

#include "mif/net/tcp_server.h"
#include "mif/net/tcp_clients.h"
#include "mif/net/client_factory.h"


#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include "mif/remote/serialization/serialization.h"
#include "mif/remote/serialization/boost/serialization.h"

#include "mif/remote/stub_client.h"
#include "mif/remote/proxy_client.h"
#include "mif/remote/ps.h"
#include "mif/service/iservice.h"
#include "mif/service/service_factory.h"

struct ITest1
    : public Mif::Service::IService
{
    virtual void SetText(std::string const &text) = 0;
    virtual std::string GetText() const = 0;
};

struct ITest2
    : public Mif::Service::IService
{
    virtual void SetName(std::string const &name) = 0;
    virtual void SetVersion(std::size_t major, std::size_t minor) = 0;
    virtual std::string GetName() const = 0;
    virtual std::size_t GetVersionMajor() const = 0;
    virtual std::size_t GetVersionMinor() const = 0;
};

class Test1 final
    : public ITest1
{
public:
    Test1()
    {
        std::cout << "Test1" << std::endl;
    }
    ~Test1()
    {
        std::cout << "~Test1" << std::endl;
    }

private:
    std::string m_text;

    // ITest1
    virtual void SetText(std::string const &text) override final
    {
        m_text = text;
    }
    virtual std::string GetText() const override final
    {
        return m_text;
    }
};

class Test2
    : public ITest2
{
public:
    Test2()
    {
        std::cout << "Test2" << std::endl;
    }
    ~Test2()
    {
        std::cout << "~Test2" << std::endl;
    }

private:
    std::string m_name = "default name";
    std::size_t m_major = 0;
    std::size_t m_minor = 0;

    // ITest2
    virtual void SetName(std::string const &name) override final
    {
        m_name = name;
    }
    virtual void SetVersion(std::size_t major, std::size_t minor) override final
    {
        m_major = major;
        m_minor = minor;
    }
    virtual std::string GetName() const override final
    {
        return m_name;
    }
    virtual std::size_t GetVersionMajor() const override final
    {
        return m_major;
    }
    virtual std::size_t GetVersionMinor() const override final
    {
        return m_minor;
    }
};

MIF_REMOTE_PS_BEGIN(ITest1)
    MIF_REMOTE_METHOD(SetText)
    MIF_REMOTE_METHOD(GetText)
MIF_REMOTE_PS_END()

MIF_REMOTE_PS_BEGIN(ITest2)
    MIF_REMOTE_METHOD(SetName)
    MIF_REMOTE_METHOD(SetVersion)
    MIF_REMOTE_METHOD(GetName)
    MIF_REMOTE_METHOD(GetVersionMajor)
    MIF_REMOTE_METHOD(GetVersionMinor)
MIF_REMOTE_PS_END()


int main(int argc, char const **argv)
{
    if (argc != 2)
    {
        std::cerr << "Bad params." << std::endl;
        return -1;
    }
	try
    {
        using BoostSerializer = Mif::Remote::Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
        using BoostDeserializer = Mif::Remote::Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
        using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<BoostSerializer, BoostDeserializer>;

        if (argv[1] == std::string("--server"))
        {
            auto serviceFactory = std::make_shared<Mif::Service::ServiceFactory>();
            serviceFactory->AddClass<Test1>("Test1");
            serviceFactory->AddClass<Test2>("Test2");

            using StubClient = Mif::Remote::StubClient<SerializerTraits, ITest1_PS, ITest2_PS>;
            using StubFactory = Mif::Net::ClientFactory<StubClient>;

            std::cout << "Creating server ..." << std::endl;
            auto netClientFactgory = std::make_shared<StubFactory>(
                std::static_pointer_cast<Mif::Service::IServiceFactory>(serviceFactory));
            auto server = std::make_shared<Mif::Net::TCPServer>(
                "localhost", "5555", 4, netClientFactgory);
            (void)server;
            std::cout << "Created server." << std::endl;
            std::cin.get();
            std::cout << "Stopping server." << std::endl;
        }
        else if (argv[1] == std::string("--client"))
        {
            using ProxyClient = Mif::Remote::ProxyClient<SerializerTraits, ITest1_PS, ITest2_PS>;
            using ProxyFactory = Mif::Net::ClientFactory<ProxyClient>;

            std::chrono::microseconds timeout{10 * 1000 * 1000};

            auto clientFactgory = std::make_shared<ProxyFactory>(timeout);
            Mif::Net::TCPClients clients(4, clientFactgory);

            auto client = std::static_pointer_cast<ProxyClient>(clients.RunClient("localhost", "5555"));

            auto test1 = client->CreateService<ITest1>("Test1");
            test1->SetText("New test text !!!");
            std::cout << "Text from test1 service: " << test1->GetText() << std::endl;

            std::cout << "Finish" << std::endl;
            std::cin.get();
        }
        else
        {
            std::cerr << "Error: waiting --server or --client" << std::endl;
            return -1;
        }

    }
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}
