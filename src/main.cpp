#include <iostream>
#include <thread>
#include <chrono>

#include "mif/net/tcp_server.h"
#include "mif/net/tcp_clients.h"
#include "mif/net/client_factory.h"
#include "mif/net/client.h"


#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include "mif/remote/ps.h"
#include "mif/remote/proxy_stub.h"
#include "mif/remote/serialization/serialization.h"
#include "mif/remote/serialization/boost/serialization.h"

void *g_m = 0;
/*
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

    using DataHandler = std::function<Mif::Common::Buffer (Mif::Common::Buffer &&)>;

    DataHandler m_handler;

    void SetHandler(DataHandler && handler)
    {
        p_transport = this;
        m_handler = std::move(handler);
    }

    Mif::Common::Buffer Send(Mif::Common::Buffer && buffer);

    Mif::Common::Buffer Call(Mif::Common::Buffer && buffer)
    {
        return m_handler(std::move(buffer));
    }
};

Mif::Common::Buffer TestTransport::Send(Mif::Common::Buffer && buffer)
{
    return p_transport->Call(std::move(buffer));
}
*/

namespace Mif
{
    namespace Remote
    {

        template
        <
            typename TInterface,
            typename TImplementation,
            template <typename, typename> class TProxyStub
        >
        struct Binder
        {
            using InterfaceType = TInterface;
            using ImplementationType = TImplementation;
            template <typename TSerializerTraits, typename TTransport>
            using ProxyStubType = TProxyStub<TSerializerTraits, TTransport>;
        };

        template
        <
            typename TSerializer,
            typename TDeserializer,
            typename TTransport,
            typename ... TBinders
        >
        class ServerFactory final
        {
        public:
        private:
        };

        struct IObjectManager
        {
            virtual ~IObjectManager() = default;
            virtual std::string CreateObject(std::string const &classId) = 0;
            virtual void DestroyObject(std::string const &instanceId) = 0;
        };

        class ObjectManager final
            : public IObjectManager
        {
        public:
        private:
            // IObjectManager
            virtual std::string CreateObject(std::string const &classId) override final
            {
                std::cout << "CreateObject. ClassId: " << classId << std::endl;
                return "new_instance";
            }

            virtual void DestroyObject(std::string const &instanceId) override final
            {
                std::cout << "DestroyObject. InstanceId: " << instanceId << std::endl;
            }
        };

        MIF_REMOTE_PS_BEGIN(IObjectManager)
            MIF_REMOTE_METHOD(CreateObject)
            MIF_REMOTE_METHOD(DestroyObject)
        MIF_REMOTE_PS_END()

        class NetClient final
            : public Net::Client
        {
        public:
            NetClient(std::weak_ptr<Net::IControl> control, std::weak_ptr<Net::IPublisher> publisher,
                bool isServerItem)
                : Client(control, publisher)
                , m_isServerItem(isServerItem)
            {
                Init();

                if (!m_isServerItem)
                {
                         std::cout << "Create client item." << std::endl;
                         g_m = m_objectManagerProxy.get();
                }
            }

            ~NetClient()
            {
                Done();
            }

            using DataHandler = std::function<Mif::Common::Buffer (Mif::Common::Buffer &&)>;

            void SetHandler(DataHandler && handler)
            {
                m_handler = std::move(handler);
            }

            Mif::Common::Buffer Send(Mif::Common::Buffer && buffer)
            {
                std::cout << (m_isServerItem ? "Server. " : "Client. ")
                             << "SendData: " << buffer.second.get() << std::endl;
                if (m_isServerItem)
                    throw std::runtime_error{"[Mif::Remote::NetClient::Send] Can't call 'Send' for server item client."};
                if (auto publisher = GetPublisher().lock())
                {
                    try
                    {
                        publisher->Publish(std::move(buffer));
                    }
                    catch (std::exception const &e)
                    {
                        std::cerr << "[Mif::Remote::NetClient::Send] Failed to send data. Error: " << e.what() << std::endl;
                        if (auto control = GetControl().lock())
                            control->CloseMe();
                        else
                            std::cerr << "[Mif::Remote::NetClient::Send] Failed to close self." << std::endl;
                    }
                }
                else
                {
                    std::cerr << "[Mif::Remote::NetClient::Send] Failed to send data. Error: no publisher." << std::endl;
                    if (auto control = GetControl().lock())
                        control->CloseMe();
                    else
                        std::cerr << "[Mif::Remote::NetClient::Send] Failed to close self." << std::endl;
                }
                for (std::size_t i = 0 ; i < 50 ; ++i)
                {
                    if (m_response.first)
                        break;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                // TODO: change on other ...
                if (!m_response.first)
                    throw std::runtime_error{"[Mif::Remote::NetClient::Send] No response."};
                return std::move(m_response);
            }

        private:
            bool m_isServerItem;
            bool m_waitResponse = false;
            Common::Buffer m_response;
            DataHandler m_handler;

            // Client
            virtual void ProcessData(Common::Buffer buffer) override final
            {
                std::cout << (m_isServerItem ? "Server. " : "Client. ")
                          << "RecvData: " << buffer.second.get() << std::endl;
                m_response = std::move(buffer);
                if (m_isServerItem)
                {
                    auto result = m_handler(std::move(std::move(m_response)));
                    if (auto publisher = GetPublisher().lock())
                    {
                        try
                        {
                            publisher->Publish(std::move(result));
                        }
                        catch (std::exception const &e)
                        {
                            std::cerr << "[Mif::Remote::NetClient::ProcessData] Failed to publish data. Error: " << e.what() << std::endl;
                            if (auto control = GetControl().lock())
                                control->CloseMe();
                            else
                                std::cerr << "[Mif::Remote::NetClient::ProcessData] Failed to close self." << std::endl;
                        }
                    }
                    else
                    {
                        std::cerr << "[Mif::Remote::NetClient::ProcessData] Failed to publish data. Error: no publisher." << std::endl;
                        if (auto control = GetControl().lock())
                            control->CloseMe();
                        else
                            std::cerr << "[Mif::Remote::NetClient::ProcessData] Failed to close self." << std::endl;
                    }
                }
            }

            class Transport final
            {
            public:
                Transport(NetClient &owner)
                    : m_owner(owner)
                {
                }

                using DataHandler = NetClient::DataHandler;

                void SetHandler(DataHandler && handler)
                {
                    m_owner.SetHandler(std::move(handler));
                }

                Mif::Common::Buffer Send(Mif::Common::Buffer && buffer)
                {
                    return m_owner.Send(std::move(buffer));
                }

            private:
                NetClient &m_owner;
            };

            friend class Transport;

            using BoostSerializer = Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
            using BoostDeserializer = Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
            using SerializerTraits = Serialization::SerializerTraits<BoostSerializer, BoostDeserializer>;
            using ObjectManagerPS = IObjectManager_PS<Proxy<SerializerTraits, Transport>, Stub<SerializerTraits, Transport>>;

            std::unique_ptr<ObjectManagerPS::Proxy> m_objectManagerProxy;
            std::unique_ptr<ObjectManagerPS::Stub> m_objectManagerStub;

            void Init()
            {
                m_objectManagerProxy.reset(new ObjectManagerPS::Proxy("0", std::move(Transport(*this))));

                auto objectManager = std::make_shared<ObjectManager>();
                m_objectManagerStub.reset(new ObjectManagerPS::Stub("0", objectManager,
                    std::move(Transport(*this))));
                m_objectManagerStub->Init();
            }

            void Done()
            {
                if (m_objectManagerStub)
                    m_objectManagerStub->Done();

                m_objectManagerProxy.reset();
                m_objectManagerStub.reset();
            }
        };

    }   // namespace Remote
}   // namespace Mif

int main()
{
	try
    {
        std::cout << "Creating server ..." << std::endl;
        auto serverFactgory = std::make_shared<Mif::Net::ClientFactory<Mif::Remote::NetClient>>(true);
        auto server = std::make_shared<Mif::Net::TCPServer>(
            "localhost", "5555", 1, serverFactgory);
        std::cout << "Created server." << std::endl;
        std::cin.get();
        std::cout << "Creating client ..." << std::endl;
        auto clientFactgory = std::make_shared<Mif::Net::ClientFactory<Mif::Remote::NetClient>>(false);
        Mif::Net::TCPClients clients(1, clientFactgory);
        std::cout << "Created client." << std::endl;
        std::cin.get();
        std::cout << "Connecting ..." << std::endl;
        clients.RunClient("localhost", "5555");
        std::cout << "Connected." << std::endl;
        std::cin.get();
        std::cout << "Try use ObjectManager ..." << std::endl;
        Mif::Remote::IObjectManager *m = (Mif::Remote::IObjectManager *)g_m;
        auto id = m->CreateObject("123456");
        for (int i = 0 ; i < 10 ; ++i)
            m->CreateObject(std::to_string(i));
        std::cout << "Created new object with id \"" << id << "\"" << std::endl;
        //m->DestroyObject(id);
        std::cin.get();

        /*using BoostSerializer = Mif::Remote::Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
        using BoostDeserializer = Mif::Remote::Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
        using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<BoostSerializer, BoostDeserializer>;
        using ProxyStub = ITest_PS<Mif::Remote::Proxy<SerializerTraits, TestTransport>, Mif::Remote::Stub<SerializerTraits, TestTransport>>;

        using ITestBindedr = Mif::Remote::Binder<ITest, Test, ITest_PS>;
        using Factory = Mif::Remote::ServerFactory<BoostSerializer, BoostDeserializer, TestTransport, ITestBindedr>;
        Factory factory;

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
        std::cout << "New minor: " << rps.GetMinor() << std::endl;*/
    }
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}

/*

int main()
{
    try
    {
        auto clientFactgory = std::make_shared<Mif::Net::ClientFactory<Mif::Net::Client>>();
        auto server = std::make_shared<Mif::Net::TCPServer>(
            "localhost", "5555", 4, clientFactgory);
        Mif::Net::TCPClients clients(4, clientFactgory);
        std::cin.get();
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
*/
