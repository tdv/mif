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
            std::chrono::microseconds timeout{10 * 1000 * 1000};
            std::cout << "Creating client ..." << std::endl;
            auto clientFactgory = std::make_shared<Mif::Net::ClientFactory<Mif::Remote::ProxyClient<SerializerTraits>>>(timeout);
            Mif::Net::TCPClients clients(4, clientFactgory);
            std::cout << "Created client." << std::endl;
            std::cout << "Connecting ..." << std::endl;
            auto client = std::static_pointer_cast<Mif::Remote::ProxyClient<SerializerTraits>>(clients.RunClient("localhost", "5555"));
            auto manager = client->CreateObjectManager();
            std::cout << "Connected." << std::endl;
            std::cout << "Try use ObjectManager ..." << std::endl;
            std::vector<std::thread> threads;
            for (int j = 0 ; j < 10 ; ++j)
            {
                std::thread t([&] ()
                        {
                            auto const tid = std::this_thread::get_id();
                            std::cout << "Begin " << tid << std::endl;
                            for (int i = 0 ; i < 10000 ; ++i)
                            {
                                try
                                {
                                    std::cout << tid << " Creating object ... " << std::endl;
                                    auto id = manager->CreateObject(std::to_string(i), "");
                                    std::cout << tid << " Created new object, Id: " << id << std::endl;
                                    std::cout << tid << " Destroing object, Id: " << id << std::endl;
                                    manager->DestroyObject(id);
                                    std::cout << tid << " Destroyed object, Id: " << id << std::endl;
                                }
                                catch (std::exception const &e)
                                {
                                    std::cerr << tid << " Error: " << e.what() << std::endl;
                                }
                            }
                            std::cout << "End " << tid << std::endl;
                        }
                    );
                threads.push_back(std::move(t));
            }

            for (auto &t : threads)
                t.join();
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
