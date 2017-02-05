//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2017 tdv
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
#include <thread>
#include <utility>

// MIF
#include "mif/common/log.h"
#include "mif/net/client.h"
#include "mif/remote/detail/iobject_manager_ps.h"
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
                , m_factory{factory}
            {
            }

            PSClient(PSClient const &) = delete;
            PSClient(PSClient &&) = delete;
            PSClient& operator = (PSClient const &) = delete;
            PSClient& operator = (PSClient &&) = delete;

            template <typename TInterface>
            Service::TServicePtr<TInterface> CreateService(std::string const &id)
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

            std::string const m_psInstanceId = "50163b9c-eba2-11e6-8863-e78765941b70";

            std::chrono::microseconds const m_timeout;
            Service::IFactoryPtr m_factory;

            std::condition_variable m_condVar;

            LockType m_lock;
            Detail::IObjectManagerPtr m_proxyObjectManager;
            Stubs m_stubs;

            LockType m_dataLock;
            Responses m_responses;

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
                        LockGuard lock{m_dataLock};
                        CleanOldResponses();
                        m_responses.insert(std::make_pair(uuid, std::move(response)));
                        m_condVar.notify_all();
                    }
                    else if (m_responses.find(uuid) == std::end(m_responses))
                    {
                        IStubPtr stub;
                        {
                            LockGuard lock(m_lock);
                            auto iter = m_stubs.find(instanceId);
                            if (iter == std::end(m_stubs))
                            {
                                if (instanceId == m_psInstanceId)
                                {
                                    using ObjectManagerStub = typename Detail::IObjectManager_PS<TSerializer>::Stub;
                                    auto manager = Service::Make<ObjectManager, ObjectManager>(this, m_factory);
                                    stub = std::make_shared<ObjectManagerStub>(manager, m_psInstanceId);
                                    m_stubs.insert(std::make_pair(m_psInstanceId, stub));
                                }
                                else
                                {
                                    throw Detail::ProxyStubException{"[Mif::Remote::PSClient::ProcessData] Instance \"" + instanceId + "\" not found."};
                                }
                            }
                            else
                            {
                                stub = iter->second;
                            }
                        }
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
            Service::IServicePtr CreateRemoteService(std::string const &serviceId)
            {
                try
                {
                    using ObjectManagerProxy = typename Detail::IObjectManager_PS<TSerializer>::Proxy;

                    auto self = std::static_pointer_cast<ThisType>(shared_from_this());
                    auto sender = std::bind(&ThisType::Send, self, std::placeholders::_1, std::placeholders::_2);

                    using PSType = typename Detail::Registry::Registry<TInterface>::template Type<TSerializer>;
                    using ProxyType = typename PSType::Proxy;

                    {
                        LockGuard lock{m_lock};

                        if (!m_proxyObjectManager)
                        {
                            m_proxyObjectManager = Service::Make<ObjectManagerProxy, Detail::IObjectManager>(m_psInstanceId, std::bind(&ThisType::Send,
                                    std::static_pointer_cast<ThisType>(shared_from_this()), std::placeholders::_1, std::placeholders::_2));
                        }
                    }

                    return Service::Make<ProxyType>(m_proxyObjectManager, serviceId, std::string{PSType::InterfaceId}, std::move(sender));
                }
                catch (std::exception const &e)
                {
                    throw Detail::ProxyStubException{"[Mif::Remote::PSClient::CreateRemoteService] "
                        "Failed to create service with id \"" + serviceId + "\". "
                        "Error: " + std::string{e.what()}};
                }
            }

            class ObjectManager
                : public Detail::IObjectManager
            {
            public:
                ObjectManager(ObjectManager const &) = delete;
                ObjectManager(ObjectManager &&) = delete;
                ObjectManager& operator = (ObjectManager const &) = delete;
                ObjectManager& operator = (ObjectManager &&) = delete;

                ObjectManager(ThisType *owner, Service::IFactoryPtr factory)
                    : m_owner{owner}
                    , m_factory{factory}
                {
                }

            private:
                ThisType *m_owner;

                Service::IFactoryPtr m_factory;
                Common::UuidGenerator m_idGenerator;

                // IObjectManager
                virtual std::string CreateObject(std::string const &serviceId, std::string const &interfaceId) override final
                {
                    auto stub = CreateStub<Detail::Registry::Counter::GetLast(Detail::FakeHierarchy{})>(serviceId, interfaceId);

                    {
                        LockGuard lock(m_owner->m_lock);
                        m_owner->m_stubs.insert(stub);
                    }
                    return stub.first;
                }

                virtual void DestroyObject(std::string const &instanceId) override final
                {
                    IStubPtr stubService;
                    {
                        LockGuard lock(m_owner->m_lock);
                        auto iter = m_owner->m_stubs.find(instanceId);
                        if (iter == std::end(m_owner->m_stubs))
                        {
                            throw std::invalid_argument{"[Mif::Remote::StubClient::DestroyObject] "
                                "Instance with id \"" + instanceId + "\" not found."};
                        }
                        stubService = std::move(iter->second);
                        m_owner->m_stubs.erase(iter);
                    }
                }

                template <std::size_t I>
                typename std::enable_if<I != 0, std::pair<std::string, IStubPtr>>::type
                CreateStub(std::string const &serviceId, std::string const &interfaceId)
                {
                    using PSType = typename Detail::Registry::template Item<I>::Type::template Type<TSerializer>;
                    if (PSType::InterfaceId == interfaceId)
                    {
                        auto const instanceId = m_idGenerator.Generate();
                        auto instance = m_factory->Create(serviceId);
                        auto stub = std::make_shared<typename PSType::Stub>(instance, instanceId);
                        return std::make_pair(instanceId, std::move(stub));
                    }
                    return CreateStub<I - 1>(serviceId, interfaceId);
                }

                template <std::size_t I>
                typename std::enable_if<I == 0, std::pair<std::string, IStubPtr>>::type
                CreateStub(std::string const &serviceId, std::string const &interfaceId)
                {
                    throw std::runtime_error{"Failed to create stub for service with id \"" + serviceId + "\". "
                        "Stub for interface with id \"" + interfaceId + "\" not found."};
                }
            };

            friend class ObjectManager;
        };

    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_PS_CLIENT_H__
