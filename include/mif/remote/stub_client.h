//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_STUB_CLIENT_H__
#define __MIF_REMOTE_STUB_CLIENT_H__

// STD
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

// MIF
#include "mif/common/uuid_generator.h"
#include "mif/net/client.h"
#include "mif/service/ifactory.h"
#include "mif/service/make.h"
#include "mif/remote/detail/iobject_manager_ps.h"

namespace Mif
{
    namespace Remote
    {

        template <typename TSerializer>
        class StubClient final
            : public Net::Client
        {
        public:
            using StubClientType = StubClient<TSerializer>;

            StubClient(StubClient const &) = delete;
            StubClient(StubClient &&) = delete;
            StubClient& operator = (StubClient const &) = delete;
            StubClient& operator = (StubClient &&) = delete;

            StubClient(std::weak_ptr<Net::IControl> control, std::weak_ptr<Net::IPublisher> publisher,
                Service::IFactoryPtr factory)
                : Client{control, publisher}
            {
                std::string const instanceId{"0"};
                auto manager = Service::Make<ObjectManager, ObjectManager>(this, factory);
                auto stub = std::make_shared<ObjectManagerStub>(manager, instanceId);
                m_stubs.insert(std::make_pair(instanceId, std::move(stub)));
            }

            ~StubClient() = default;

        private:
            using IStubPtr = std::shared_ptr<Detail::IStub<TSerializer>>;
            using Stubs = std::map<std::string/*instance id*/, IStubPtr>;
            using Serializer = typename TSerializer::Serializer;
            using Deserializer = typename TSerializer::Deserializer;
            using ObjectManagerStub = typename Detail::IObjectManager_PS<TSerializer>::Stub;

            using LockType = std::mutex;
            using LockGuard = std::lock_guard<LockType>;

            LockType m_lock;
            Stubs m_stubs;


            // Client
            virtual void ProcessData(Common::Buffer buffer) override final
            {
                try
                {
                    if (buffer.empty())
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
                        auto iter = m_stubs.find(instanceId);
                        if (iter == std::end(m_stubs))
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
                    stub->Call(deserializer, serializer);
                    if (!Post(std::move(serializer.GetBuffer())))
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

            class ObjectManager
                : public Detail::IObjectManager
            {
            public:
                ObjectManager(ObjectManager const &) = delete;
                ObjectManager(ObjectManager &&) = delete;
                ObjectManager& operator = (ObjectManager const &) = delete;
                ObjectManager& operator = (ObjectManager &&) = delete;

                ObjectManager(StubClientType *owner, Service::IFactoryPtr factory)
                    : m_owner{owner}
                    , m_factory{factory}
                {
                }

            private:
                StubClientType *m_owner;

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

#endif  // !__MIF_REMOTE_STUB_CLIENT_H__
