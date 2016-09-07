#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include <mutex>
#include <ctime>

#include "mif/net/tcp_server.h"
#include "mif/net/tcp_clients.h"
#include "mif/net/client_factory.h"


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

                std::cout << "StubClient" << std::endl;
            }

            ~StubClient()
            {
                std::cout << "~StubClient" << std::endl;
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
                        throw Detail::ProxyStubException{"[Mif::Remote::StubClient::ProcessData] Empty interface id for instanse "
                            "\"" + instanceId + "\""};
                    }
                    auto const &method = deserializer.GetMethod();
                    if (method.empty())
                        throw Detail::ProxyStubException{"[Mif::Remote::StubClient::ProcessData] Empty method name of interface \"" + interfaceId  + "\""};
                    Serializer serializer(false, deserializer.GetUuid(), instanceId, interfaceId, method);
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

        template <typename TSerializer>
        class ProxyClient
            : public Net::Client
        {
        public:
            using ThisType = ProxyClient<TSerializer>;

            ProxyClient(std::weak_ptr<Net::IControl> control, std::weak_ptr<Net::IPublisher> publisher)
                : Client(control, publisher)
            {
            }

            using IObjectManagerPtr = std::shared_ptr<Detail::IObjectManager>;

            IObjectManagerPtr CreateObjectManager()
            {
                auto proxy = std::make_shared<ObjectManagerProxy>(std::string{"0"}, std::bind(&ThisType::Send,
                    std::static_pointer_cast<ThisType>(shared_from_this()), std::placeholders::_1, std::placeholders::_2));
                return std::static_pointer_cast<Detail::IObjectManager>(proxy);
            }

        private:
            using Serializer = typename TSerializer::Serializer;
            using Deserializer = typename TSerializer::Deserializer;
            using ObjectManagerProxy = typename Detail::IObjectManager_PS<TSerializer>::Proxy;

            using DeserializerPtr = std::unique_ptr<Deserializer>;
            using Response = std::pair<std::time_t/*timestamp*/, DeserializerPtr>;
            using Responses = std::map<std::string/*uuid*/, Response>;
            Responses m_responses;

            DeserializerPtr Send(std::string const &requestId, Serializer &serializer)
            {
                if (!Post(std::move(serializer.GetBuffer())))
                {
                    if (!CloseMe())
                    {
                        throw Detail::ProxyStubException{"[Mif::Remote::ProxyClient::Send] Failed to post request."
                            "No channel for post data and failed to close self."};
                    }
                    throw Detail::ProxyStubException{"[Mif::Remote::ProxyClient::Send] Failed to post request. "
                        "No channel for post data."};
                }

                // TODO: do other wait
                for (int i = 0 ; i < 1000 ; ++i)
                {
                    auto iter = m_responses.find(requestId);
                    if (iter != std::end(m_responses))
                    {
                        auto deserializer = std::move(iter->second.second);
                        m_responses.erase(iter);
                        return std::move(deserializer);
                    }
                    std::this_thread::sleep_for(std::chrono::microseconds(1000));
                }

                throw Detail::ProxyStubException{"[Mif::Remote::ProxyClient::Send] Failed to send data. "
                    "Expired response timeout from remote server."};
            }

            virtual void ProcessData(Common::Buffer buffer) override final
            {
                try
                {
                    if (!buffer.first)
                        throw Detail::ProxyStubException{"[Mif::Remote::ProxyClient::ProcessData] Empty data."};
                    DeserializerPtr deserializer{new Deserializer(std::move(buffer))};
                    if (!deserializer->IsResponse())
                        throw Detail::ProxyStubException{"[Mif::Remote::ProxyClient::ProcessData] Bad response type \"" + deserializer->GetType() + "\""};
                    auto const &instanceId = deserializer->GetInstance();
                    if (instanceId.empty())
                        throw Detail::ProxyStubException{"[Mif::Remote::ProxyClient::ProcessData] Empty instance id."};
                    auto const &interfaceId = deserializer->GetInterface();
                    if (interfaceId.empty())
                    {
                        throw Detail::ProxyStubException{"[Mif::Remote::ProxyClient::ProcessData] Empty interface id for instanse "
                            "\"" + instanceId + "\""};
                    }
                    auto const &method = deserializer->GetMethod();
                    if (method.empty())
                    {
                        throw Detail::ProxyStubException{"[Mif::Remote::ProxyClient::ProcessData] Empty method name of interface "
                            "\"" + interfaceId  + "\" for instance \"" + instanceId + "\""};
                    }
                    auto const uuid = deserializer->GetUuid();
                    if (m_responses.find(uuid) != std::end(m_responses))
                    {
                        throw Detail::ProxyStubException{"[Mif::Remote::ProxyClient::ProcessData] Response id "
                            "\"" + uuid + "\" not unique."};
                    }
                    Response response{std::time(nullptr), std::move(deserializer)};
                    m_responses.insert(std::make_pair(uuid, std::move(response)));
                }
                catch (Detail::ProxyStubException const &)
                {
                    throw;
                }
                catch (std::exception const &e)
                {
                    throw Detail::ProxyStubException{"[Mif::Remote::ProxyClient::ProcessData] "
                        "Failed to process data. Error: " + std::string{e.what()}};
                }
                catch (...)
                {
                    throw Detail::ProxyStubException{"[Mif::Remote::ProxyClient::ProcessData] "
                        "Failed to process data. Error: unknown."};
                }
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
        using BoostSerializer = Mif::Remote::Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
        using BoostDeserializer = Mif::Remote::Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
        using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<BoostSerializer, BoostDeserializer>;

        if (argv[1] == std::string("--server"))
        {
            std::cout << "Creating server ..." << std::endl;
            auto serverFactgory = std::make_shared<Mif::Net::ClientFactory<Mif::Remote::StubClient<SerializerTraits>>>();
            auto server = std::make_shared<Mif::Net::TCPServer>(
                "localhost", "5555", 4, serverFactgory);
            (void)server;
            std::cout << "Created server." << std::endl;
            std::cin.get();
            std::cout << "Stopping server." << std::endl;
        }
        else if (argv[1] == std::string("--client"))
        {
            std::cout << "Creating client ..." << std::endl;
            auto clientFactgory = std::make_shared<Mif::Net::ClientFactory<Mif::Remote::ProxyClient<SerializerTraits>>>();
            Mif::Net::TCPClients clients(4, clientFactgory);
            std::cout << "Created client." << std::endl;
            std::cout << "Connecting ..." << std::endl;
            auto client = std::static_pointer_cast<Mif::Remote::ProxyClient<SerializerTraits>>(clients.RunClient("localhost", "5555"));
            auto manager = client->CreateObjectManager();
            std::cout << "Connected." << std::endl;
            std::cout << "Try use ObjectManager ..." << std::endl;
            for (int i = 0 ; i < 10 ; ++i)
            {
                auto id = manager->CreateObject(std::to_string(i));
                manager->DestroyObject(id);
            }
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
