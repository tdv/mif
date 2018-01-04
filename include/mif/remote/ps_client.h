//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_PS_CLIENT_H__
#define __MIF_REMOTE_PS_CLIENT_H__

// STD
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <sstream>
#include <thread>
#include <utility>

// MIF
#include "mif/net/client.h"
#include "mif/remote/detail/meta/iobject_manager.h"
#include "mif/remote/meta/iservice.h"
#include "mif/service/factory.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Remote
    {

        template <typename TSerializer>
        class PSClient final
            : public Net::Client
        {
        public:
            using ThisType = PSClient<TSerializer>;

            PSClient(std::weak_ptr<Net::IControl> control, std::weak_ptr<Net::IPublisher> publisher,
                std::chrono::microseconds const &timeout,
                Service::IFactoryPtr factory = Service::Make<Service::Factory, Service::IFactory>())
                : Client{control, publisher}
                , m_timeout{timeout}
            {
                using ObjectManagerStub = typename Detail::Meta::IObjectManager_PS<TSerializer>::Stub;
                m_stubObjectManager = Service::Make<ObjectManager, ObjectManager>(this, std::move(factory));
                auto stub = std::make_shared<ObjectManagerStub>(m_stubObjectManager, m_psInstanceId);
                m_stubs.insert(std::make_pair(m_psInstanceId, std::move(stub)));
            }

            PSClient(PSClient const &) = delete;
            PSClient(PSClient &&) = delete;
            PSClient& operator = (PSClient const &) = delete;
            PSClient& operator = (PSClient &&) = delete;

            template <typename TInterface>
            Service::TServicePtr<TInterface> CreateService(Service::ServiceId id)
            {
                return Service::Cast<TInterface>(CreateRemoteService<TInterface>(id));
            }

        private:
            using LockType = std::mutex;
            using LockGuard = std::lock_guard<LockType>;

            using Serializer = typename TSerializer::Serializer;
            using Deserializer = typename TSerializer::Deserializer;

            using DeserializerPtr = std::unique_ptr<Deserializer>;
            using Response = std::pair<std::chrono::microseconds/*timestamp*/, DeserializerPtr>;
            using Responses = std::map<std::string/*uuid*/, Response>;

            using IStubPtr = std::shared_ptr<Detail::IStub<TSerializer>>;
            using Stubs = std::map<std::string/*instance id*/, IStubPtr>;

            class ObjectManager
                : public Service::Inherit<Detail::IObjectManager>
            {
            public:
                ObjectManager(ThisType *owner, Service::IFactoryPtr factory)
                    : m_owner{owner}
                    , m_factory{factory}
                {
                }

                Detail::StubCreator GetStubCreator()
                {
                    auto self = Service::TServicePtr<ObjectManager>{this};
                    auto creator = std::bind(&ObjectManager::AppendStub, self,
                            std::placeholders::_1, std::placeholders::_2);
                    return creator;
                }

            private:
                Common::UuidGenerator m_uuidGenerator;

                ThisType *m_owner;

                Service::IFactoryPtr m_factory;

                // IObjectManager
                virtual std::string CreateObject(Service::ServiceId serviceId, std::string const &interfaceId) override final
                {
                    if (interfaceId.empty())
                        throw std::invalid_argument{"[Mif::Remote::PSClient::CreateObject] Parameter \"interfaceId\" must not be empty."};

                    auto instance = m_factory->Create(serviceId);
                    return AppendStub(std::move(instance), interfaceId);
                }

                virtual void DestroyObject(std::string const &instanceId) override final
                {
                    if (instanceId.empty())
                        throw std::invalid_argument{"[Mif::Remote::PSClient::DestrowObject] Parameter \"instanceId\" must not be empty."};

                    IStubPtr stubService;
                    {
                        LockGuard lock(m_owner->m_lock);
                        auto iter = m_owner->m_stubs.find(instanceId);
                        if (iter == std::end(m_owner->m_stubs))
                        {
                            throw std::invalid_argument{"[Mif::Remote::PSClient::DestroyObject] "
                                "Instance with id \"" + instanceId + "\" not found."};
                        }
                        stubService = std::move(iter->second);
                        m_owner->m_stubs.erase(iter);
                    }
                }

                virtual std::string QueryInterface(std::string const &instanceId, std::string const &interfaceId,
                        std::string const &serviceId) override final
                {
                    IStubPtr stub;
                    {
                        LockGuard lock(m_owner->m_lock);
                        auto iter = m_owner->m_stubs.find(instanceId);
                        if (iter == std::end(m_owner->m_stubs))
                        {
                            throw std::invalid_argument{"[Mif::Remote::PSClient::QueryInterface] "
                                "Instance with id \"" + instanceId + "\" not found."};
                        }
                        stub = iter->second;
                    }

                    auto instance = stub->Query(interfaceId, serviceId);
                    if (!instance)
                        return {};

                    return AppendStub(std::move(instance), interfaceId);
                }

                virtual std::string CloneReference(std::string const &instanceId,
                        std::string const &interfaceId) override final
                {
                    Service::IServicePtr instance;
                    {
                        LockGuard lock(m_owner->m_lock);
                        auto iter = m_owner->m_stubs.find(instanceId);
                        if (iter == std::end(m_owner->m_stubs))
                        {
                            throw std::invalid_argument{"[Mif::Remote::PSClient::CloneReference] "
                                "Instance with id \"" + instanceId + "\" not found."};
                        }
                        instance = iter->second->GetInstance();
                    }
                    auto newInstanceId = AppendStub(std::move(instance), interfaceId);
                    return newInstanceId;
                }

                struct CreateStubVisitor
                {
                    using Serializer = TSerializer;
                    using Result = IStubPtr;

                    template <typename T>
                    static Result Visit(ObjectManager *objectManager, Service::IServicePtr instance,
                        std::string const &instanceId, std::string const &interfaceId)
                    {
                        if (T::InterfaceId == interfaceId)
                        {
                            auto self = Service::TServicePtr<ObjectManager>{objectManager};
                            auto stubCreator = objectManager->GetStubCreator();
                            auto sender = std::bind(&ThisType::Send, objectManager->m_owner,
                                    std::placeholders::_1, std::placeholders::_2);
                            return std::make_shared<typename T::Stub>(std::move(instance), instanceId,
                                    objectManager->m_owner->GetProxyObjectManager(),
                                    std::move(stubCreator), std::move(sender));
                        }
                        return {};
                    }
                };

                std::string AppendStub(Service::IServicePtr instance, std::string const &interfaceId)
                {
                    if (!instance)
                        return {};

                    auto const instanceId = m_uuidGenerator.Generate();

                    {
                        LockGuard lock(m_owner->m_lock);
                        if (m_owner->m_stubs.find(instanceId) != std::end(m_owner->m_stubs))
                            return instanceId;
                    }


                    auto stub = Detail::Registry::Visitor::Accept<CreateStubVisitor>(this,
                            std::move(instance), instanceId, interfaceId);

                    {
                        LockGuard lock(m_owner->m_lock);

                        if (m_owner->m_stubs.find(instanceId) != std::end(m_owner->m_stubs))
                            return instanceId;

                        m_owner->m_stubs.insert(std::make_pair(instanceId, std::move(stub)));
                    }

                    return instanceId;
            }

            };

            friend class ObjectManager;

            std::string const m_psInstanceId = "0";

            std::chrono::microseconds const m_timeout;

            std::condition_variable m_condVar;

            LockType m_lock;
            Detail::IObjectManagerPtr m_proxyObjectManager;
            Service::TIntrusivePtr<ObjectManager> m_stubObjectManager;

            Stubs m_stubs;

            LockType m_dataLock;
            Responses m_responses;

            Detail::IObjectManagerPtr GetProxyObjectManager()
            {
                LockGuard lock{m_lock};
                if (!m_proxyObjectManager)
                {
                    using ObjectManagerProxy = typename Detail::Meta::IObjectManager_PS<TSerializer>::Proxy;
                    m_proxyObjectManager = Service::Make<ObjectManagerProxy, Detail::IObjectManager>(m_psInstanceId, std::bind(&ThisType::Send,
                            std::static_pointer_cast<ThisType>(shared_from_this()), std::placeholders::_1, std::placeholders::_2),
                            [] (Service::IServicePtr, std::string const &interfaceId) -> std::string
                            {
                                throw Detail::ProxyStubException{"[Mif::Remote::PSClient::GetProxyObjectManager] "
                                    "Failed to create proxy from ObjectManager. Interface id \"" + interfaceId + "\""};
                            }
                        );
                }
                return m_proxyObjectManager;
            }

            DeserializerPtr Send(std::string const &requestId, Serializer &serializer)
            {
                if (!Post(std::move(serializer.GetBuffer())))
                {
                    if (!CloseMe())
                    {
                        throw Detail::ProxyStubException{"[Mif::Remote::PSClient::Send] Failed to post request. "
                            "No channel for post data and failed to close self."};
                    }
                    throw Detail::ProxyStubException{"[Mif::Remote::PSClient::Send] Failed to post request. "
                        "No channel for post data."};
                }

                for (auto t = GetCurTime() ; (GetCurTime() - t) < m_timeout ; )
                {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));

                    {
                        std::unique_lock<LockType> lock{m_dataLock};
                        typename Responses::iterator iter = std::end(m_responses);
                        auto waitResult = m_condVar.wait_for(lock, m_timeout,
                                [this, &requestId, &iter] ()
                                {
                                    iter = m_responses.find(requestId);
                                    return iter != std::end(m_responses) || IsClosed();
                                }
                            );
                        if (IsClosed())
                        {
                            throw Detail::ProxyStubException{"[Mif::Remote::PSClient::Send] Failed to send data. "
                                "Connection was closed by remote server."};
                        }
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

                throw Detail::ProxyStubException{"[Mif::Remote::PSClient::Send] Failed to send data. "
                    "Expired response timeout from remote server."};
            }

            // Client
            virtual void ProcessData(Common::Buffer buffer) override final
            {
                try
                {
                    if (buffer.empty())
                        throw Detail::ProxyStubException{"[Mif::Remote::PSClient::ProcessData] Empty data."};
                    DeserializerPtr deserializer{new Deserializer(std::move(buffer))};
                    auto const uuid = deserializer->GetUuid();
                    if (uuid.empty())
                        throw Detail::ProxyStubException{"[Mif::Remote::PSClient::ProcessData] Empty package id (no uuid)."};
                    auto const &instanceId = deserializer->GetInstance();
                    if (instanceId.empty())
                        throw Detail::ProxyStubException{"[Mif::Remote::PSClient::ProcessData] Empty instance id."};
                    auto const &interfaceId = deserializer->GetInterface();
                    if (interfaceId.empty())
                    {
                        throw Detail::ProxyStubException{"[Mif::Remote::PSClient::ProcessData] Empty interface id for instanse "
                            "\"" + instanceId + "\""};
                    }
                    auto const &method = deserializer->GetMethod();
                    if (method.empty())
                    {
                        throw Detail::ProxyStubException{"[Mif::Remote::PSClient::ProcessData] Empty method name of interface "
                            "\"" + interfaceId  + "\" for instance \"" + instanceId + "\""};
                    }
                    if (deserializer->IsResponse())
                    {
                        Response response{GetCurTime(), std::move(deserializer)};
                        {
                            LockGuard lock{m_dataLock};
                            CleanOldResponses();
                            m_responses.insert(std::make_pair(uuid, std::move(response)));
                        }
                        m_condVar.notify_all();
                    }
                    else if (m_responses.find(uuid) == std::end(m_responses))
                    {
                        IStubPtr stub;
                        {
                            LockGuard lock(m_lock);
                            auto iter = m_stubs.find(instanceId);
                            if (iter != std::end(m_stubs))
                                stub = iter->second;
                        }
                        if (!stub)
                            throw Detail::ProxyStubException{"[Mif::Remote::PSClient::ProcessData] Instance \"" + instanceId + "\" not found."};
                        Serializer serializer(false, uuid, instanceId, interfaceId, method);
                        stub->Call(*deserializer, serializer);
                        if (!Post(std::move(serializer.GetBuffer())))
                        {
                            if (!CloseMe())
                            {
                                throw Detail::ProxyStubException{"[Mif::Remote::PSClient::ProcessData] Failed to post response from instance "
                                    "\"" + instanceId + "\". No channel for post data and failed to close self."};
                            }
                            throw Detail::ProxyStubException{"[Mif::Remote::PSClient::ProcessData] Failed to post response from instanse "
                                "\"" + instanceId + "\". No channel for post data."};
                        }
                    }
                    else
                    {
                        throw Detail::ProxyStubException{"[Mif::Remote::PSClient::ProcessData] Response id "
                            "\"" + uuid + "\" not unique."};
                    }
                }
                catch (Detail::ProxyStubException const &)
                {
                    throw;
                }
                catch (std::exception const &e)
                {
                    throw Detail::ProxyStubException{"[Mif::Remote::PSClient::ProcessData] "
                        "Failed to process data. Error: " + std::string{e.what()}};
                }
                catch (...)
                {
                    throw Detail::ProxyStubException{"[Mif::Remote::PSClient::ProcessData] "
                        "Failed to process data. Error: unknown."};
                }
            }

            virtual void Close() override final
            {
                m_condVar.notify_all();
            }

            std::chrono::microseconds GetCurTime() const
            {
                return std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::system_clock::now().time_since_epoch());
            }

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

            template <typename TInterface>
            Service::IServicePtr CreateRemoteService(Service::ServiceId serviceId)
            {
                try
                {
                    auto self = std::static_pointer_cast<ThisType>(shared_from_this());
                    auto sender = std::bind(&ThisType::Send, self, std::placeholders::_1, std::placeholders::_2);
                    auto stubCreator = m_stubObjectManager->GetStubCreator();

                    using PSType = typename Detail::Registry::Registry<TInterface>::template Type<TSerializer>;
                    using ProxyType = typename PSType::Proxy;

                    return Service::Make<ProxyType>(GetProxyObjectManager(), serviceId, std::string{PSType::InterfaceId},
                            std::move(sender), std::move(stubCreator));
                }
                catch (std::exception const &e)
                {
                    throw Detail::ProxyStubException{"[Mif::Remote::PSClient::CreateRemoteService] "
                        "Failed to create service with id \"" + std::to_string(serviceId) + "\". "
                        "Error: " + std::string{e.what()}};
                }
            }
        };

    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_PS_CLIENT_H__
