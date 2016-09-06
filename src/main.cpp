#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include <mutex>

#include "mif/net/tcp_server.h"
#include "mif/net/tcp_clients.h"
#include "mif/net/client_factory.h"
#include "mif/net/client.h"


#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "mif/remote/ps.h"
#include "mif/remote/serialization/serialization.h"
#include "mif/remote/serialization/boost/serialization.h"

void *g_m = 0;

namespace Mif
{
    namespace Remote
    {

        /*class NetClient
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

        private:
            using DataHandler = std::function<Mif::Common::Buffer (Mif::Common::Buffer &&)>;

            bool m_isServerItem;
            bool m_waitResponse = false;
            Common::Buffer m_response;
            DataHandler m_handler;

            void SetHandler(DataHandler && handler)
            {
                m_handler = std::move(handler);
            }

            Mif::Common::Buffer Send(Mif::Common::Buffer && buffer)
            {
                if (m_isServerItem)
                    throw std::runtime_error{"[Mif::Remote::NetClient::Send] Can't call 'Send' for server item client."};
                try
                {
                    if (!Post(std::move(buffer)))
                    {
                        std::cerr << "[Mif::Remote::NetClient::Send] Failed to send data. Error: no publisher." << std::endl;
                        if (!CloseMe())
                            std::cerr << "[Mif::Remote::NetClient::Send] Failed to close self." << std::endl;
                    }
                }
                catch (std::exception const &e)
                {
                    std::cerr << "[Mif::Remote::NetClient::Send] Failed to send data. Error: " << e.what() << std::endl;
                    if (!CloseMe())
                        std::cerr << "[Mif::Remote::NetClient::Send] Failed to close self." << std::endl;
                }
                for (std::size_t i = 0 ; i < 5000 ; ++i)
                {
                    if (m_response.first)
                        break;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                // TODO: change on other ...
                if (!m_response.first)
                    throw std::runtime_error{"[Mif::Remote::NetClient::Send] No response."};
                auto result = std::move(m_response);
                Common::Buffer{}.swap(m_response);
                return result;
            }

            // Client
            virtual void ProcessData(Common::Buffer buffer) override final
            {
                m_response = std::move(buffer);
                if (m_isServerItem)
                {
                    auto data = std::move(m_response);
                    Common::Buffer{}.swap(m_response);
                    auto result = m_handler(std::move(std::move(data)));
                    try
                    {
                        if (!Post(std::move(result)))
                        {
                            std::cerr << "[Mif::Remote::NetClient::ProcessData] Failed to publish data. Error: no publisher." << std::endl;
                            if (!CloseMe())
                                std::cerr << "[Mif::Remote::NetClient::ProcessData] Failed to close self." << std::endl;
                        }
                    }
                    catch (std::exception const &e)
                    {
                        std::cerr << "[Mif::Remote::NetClient::ProcessData] Failed to publish data. Error: " << e.what() << std::endl;
                        if (!CloseMe())
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
            //using BoostSerializer = Serialization::Boost::Serializer<boost::archive::binary_oarchive>;
            //using BoostDeserializer = Serialization::Boost::Deserializer<boost::archive::binary_iarchive>;
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
        };*/

        class ProxyNetClient
            : public Net::Client
        {
        public:
            using Client::Client;
        private:
        };

        namespace Detail
        {

            struct IObjectManager
            {
                virtual ~IObjectManager() = default;
                virtual std::string CreateObject(std::string const &classId) = 0;
                virtual void DestroyObject(std::string const &instanceId) = 0;
            };

            MIF_REMOTE_PS_BEGIN(IObjectManager)
                MIF_REMOTE_METHOD(CreateObject)
                MIF_REMOTE_METHOD(DestroyObject)
            MIF_REMOTE_PS_END()

        }   // namespace Detail

        template <typename TSerializer>
        class StubClient final
            : public Net::Client
            , public Detail::IObjectManager
        {
        public:
            StubClient(std::weak_ptr<Net::IControl> control, std::weak_ptr<Net::IPublisher> publisher)
                : Client(control, publisher)
            {
                m_instances.insert(std::make_pair(std::string{"0"}, std::make_shared<ObjectManagerStub>()));
            }

        private:
            using IStubPtr = std::shared_ptr<Detail::IStub<TSerializer>>;
            using Instances = std::map<std::string/*instance id*/, IStubPtr>;
            using Serializer = typename TSerializer::Serializer;
            using Deserializer = typename TSerializer::Deserializer;
            using ObjectManagerStub = typename Detail::IObjectManager_PS<TSerializer>::Stub;

            using LockType = std::mutex;
            using LockGuard = std::lock_guard<LockType>;

            LockType m_lock;
            Instances m_instances;


            // Client
            virtual void ProcessData(Common::Buffer buffer) override final
            {
                try
                {
                    if (!buffer.first)
                        throw Detail::ProxyStubException{"[Mif::Remote::StubClient::ProcessData] Empty data."};
                    Deserializer deserializer(std::move(buffer));
                    if (!deserializer.IsRequest())
                        throw Detail::ProxyStubException{"[Mif::Remote::StubClient::ProcessData] Bad request type \"" + deserializer.GetType() + "\""};
                    auto const &instanceId = deserializer.GetInstance();
                    if (instanceId.empty())
                        throw Detail::ProxyStubException{"[Mif::Remote::StubClient::ProcessData] Empty instance id."};
                    IStubPtr stub;
                    {
                        LockGuard lock(m_lock);
                        auto iter = m_instances.find(instanceId);
                        if (iter == std::end(m_instances))
                            throw Detail::ProxyStubException{"[Mif::Remote::StubClient::ProcessData] Instance \"" + instanceId + "\" not found."};
                        stub = iter->second;
                    }
                    auto const &interfaceId = deserializer.GetInterface();
                    if (interfaceId.empty())
                    {
                        throw Detail::ProxyStubException{"[Mif::Remote::StubClient::ProcessData] Empty method interface id for instanse "
                            "\"" + instanceId + "\""};
                    }
                    auto const &method = deserializer.GetMethod();
                    if (method.empty())
                        throw Detail::ProxyStubException{"[Mif::Remote::StubClient::ProcessData] Empty method name of interface \"" + interfaceId  + "\""};
                    Serializer serializer(instanceId, interfaceId, method, false);
                    stub->Call(static_cast<Detail::IObjectManager *>(this), deserializer, serializer);
                    if (!Post(serializer.GetBuffer()))
                    {
                        if (!CloseMe())
                        {
                            throw Detail::ProxyStubException{"[Mif::Remote::StubClient::ProcessData] Failed to post response from instance "
                                "\"" + instanceId + "\". No channel for post data and failed to close self."};
                        }
                        throw Detail::ProxyStubException{"[Mif::Remote::StubClient::ProcessData] Failed to post response from instanse "
                            "\"" + instanceId + "\". No channel for post data."};
                    }
                }
                catch (Detail::ProxyStubException const &)
                {
                    throw;
                }
                catch (std::exception const &e)
                {
                    throw Detail::ProxyStubException{"[Mif::Remote::StubClient::ProcessData] "
                        "Failed to process data. Error: " + std::string{e.what()}};
                }
                catch (...)
                {
                    throw Detail::ProxyStubException{"[Mif::Remote::StubClient::ProcessData] "
                        "Failed to process data. Error: unknown."};
                }
            }

            // IObjectManager
            virtual std::string CreateObject(std::string const &classId) override final
            {
                std::cout << "CreateObject. ClassId: " << classId << std::endl;
                return "new_instance_" + classId;
            }

            virtual void DestroyObject(std::string const &instanceId) override final
            {
                std::cout << "DestroyObject. InstanceId: " << instanceId << std::endl;
            }
        };

    }   // namespace Remote
}   // namespace Mif

int main(int argc, char const **argv)
{
    if (argc != 2)
    {
        std::cerr << "Bad params." << std::endl;
        return -1;
    }
	try
    {
        if (argv[1] == std::string("--server"))
        {
            std::cout << "Creating server ..." << std::endl;
            using BoostSerializer = Mif::Remote::Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
            using BoostDeserializer = Mif::Remote::Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
            using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<BoostSerializer, BoostDeserializer>;
            auto serverFactgory = std::make_shared<Mif::Net::ClientFactory<Mif::Remote::StubClient<SerializerTraits>>>();
            auto server = std::make_shared<Mif::Net::TCPServer>(
                "localhost", "5555", 4, serverFactgory);
            (void)server;
            std::cout << "Created server." << std::endl;
            std::cin.get();
        }
        /*else if (argv[1] == std::string("--client"))
        {
            std::cout << "Creating client ..." << std::endl;
            auto clientFactgory = std::make_shared<Mif::Net::ClientFactory<Mif::Remote::ProxyNetClient>>(false);
            Mif::Net::TCPClients clients(4, clientFactgory);
            std::cout << "Created client." << std::endl;
            std::cout << "Connecting ..." << std::endl;
            clients.RunClient("localhost", "5555");
            std::cout << "Connected." << std::endl;
            std::cout << "Try use ObjectManager ..." << std::endl;
            Mif::Remote::IObjectManager *m = (Mif::Remote::IObjectManager *)g_m;
            auto id = m->CreateObject("123456");
            for (int i = 0 ; i < 100 ; ++i)
                m->CreateObject(std::to_string(i));
            std::cout << "Created new object with id \"" << id << "\"" << std::endl;
            m->DestroyObject(id);
            std::cin.get();
        }*/
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
