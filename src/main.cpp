#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include <mutex>
#include <condition_variable>

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

#include <unistd.h>
#include <signal.h>

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
                std::this_thread::sleep_for(std::chrono::microseconds(50000));
                //std::cout << "CreateObject. ClassId: " << classId << std::endl;
                return "new_instance_" + classId;
            }

            virtual void DestroyObject(std::string const &instanceId) override final
            {
                //std::cout << "DestroyObject. InstanceId: " << instanceId << std::endl;
            }
        };

        template <typename TSerializer>
        class ProxyClient
            : public Net::Client
        {
        public:
            using ThisType = ProxyClient<TSerializer>;

            ProxyClient(std::weak_ptr<Net::IControl> control, std::weak_ptr<Net::IPublisher> publisher,
                std::chrono::microseconds const &timeout)
                : Client{control, publisher}
                , m_timeout{timeout}
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
            using Response = std::pair<std::chrono::microseconds/*timestamp*/, DeserializerPtr>;
            using Responses = std::map<std::string/*uuid*/, Response>;

            std::chrono::microseconds const m_timeout;
            std::condition_variable m_condVar;
            std::mutex m_mtx;
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

                for (auto t = GetCurTime() ; (GetCurTime() - t) < m_timeout ; )
                {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));

                    {
                        std::unique_lock<std::mutex> lock(m_mtx);
                        typename Responses::iterator iter = std::end(m_responses);
                        auto waitResult = m_condVar.wait_for(lock, m_timeout,
                                [this, &requestId, &iter] ()
                                {
                                    iter = m_responses.find(requestId);
                                    return iter != std::end(m_responses);
                                }
                            );
                        if (waitResult)
                        {
                            auto deserializer = std::move(iter->second.second);
                            m_responses.erase(iter);

                            CleanOldResponses();

                            return std::move(deserializer);
                        }

                        CleanOldResponses();
                    }

                    std::this_thread::sleep_for(std::chrono::microseconds(5));
                }

                throw Detail::ProxyStubException{"[Mif::Remote::ProxyClient::Send] Failed to send data. "
                    "Expired response timeout from remote server."};
            }

            // Client
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
                    Response response{GetCurTime(), std::move(deserializer)};
                    std::lock_guard<std::mutex> lock(m_mtx);
                    CleanOldResponses();
                    m_responses.insert(std::make_pair(uuid, std::move(response)));
                    m_condVar.notify_all();
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

            std::chrono::microseconds GetCurTime() const
            {
                return std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::system_clock::now().time_since_epoch());
            };

            void CleanOldResponses()
            {
                auto const now = GetCurTime();
                for (auto i = std::begin(m_responses) ; i != std::end(m_responses) ; )
                {
                    if (i->second.first < now && (now - i->second.first) > m_timeout)
                        m_responses.erase(i++);
                    else
                        ++i;
                }
            }
        };

    }   // namespace Remote
}   // namespace Mif

int main(int argc, char const **argv)
{
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        throw std::runtime_error("[Application::onStart] Failed to set ignore SIGPIPE.");
    }

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
                                    auto id = manager->CreateObject(std::to_string(i));
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
