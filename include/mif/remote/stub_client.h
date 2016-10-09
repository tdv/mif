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
#include "mif/service/iservice_factory.h"
#include "mif/remote/detail/iobject_manager_ps.h"

namespace Mif
{
    namespace Remote
    {

        template
        <
            typename TSerializer,
            template <typename> class ... TProxyStubs
        >
        class StubClient final
            : public Net::Client
        {
        public:
            using StubClientType = StubClient<TSerializer, TProxyStubs ... >;

            StubClient(StubClient const &) = delete;
            StubClient(StubClient &&) = delete;
            StubClient& operator = (StubClient const &) = delete;
            StubClient& operator = (StubClient &&) = delete;

            StubClient(std::weak_ptr<Net::IControl> control, std::weak_ptr<Net::IPublisher> publisher,
                Service::IServiceFactoryPtr serviceFactory)
                : Client{control, publisher}
            {
                m_stubs.insert(std::make_pair(std::string{"0"},
                    std::make_pair(std::make_shared<ObjectManagerStub>(),
                    std::make_shared<ObjectManager>(this, serviceFactory))));
            }

            ~StubClient() = default;

        private:
            using IStubPtr = std::shared_ptr<Detail::IStub<TSerializer>>;
            using StubService = std::pair<IStubPtr, Service::IServicePtr>;
            using Stubs = std::map<std::string/*instance id*/, StubService>;
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
                    Service::IServicePtr service;
                    {
                        LockGuard lock(m_lock);
                        auto iter = m_stubs.find(instanceId);
                        if (iter == std::end(m_stubs))
                            throw Detail::ProxyStubException{"[Mif::Remote::StubClient::ProcessData] Instance \"" + instanceId + "\" not found."};
                        stub = iter->second.first;
                        service = iter->second.second;
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
                    stub->Call(service.get(), deserializer, serializer);
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

            class ObjectManager final
                : public Detail::IObjectManager
            {
            public:
                ObjectManager(ObjectManager const &) = delete;
                ObjectManager(ObjectManager &&) = delete;
                ObjectManager& operator = (ObjectManager const &) = delete;
                ObjectManager& operator = (ObjectManager &&) = delete;

                ObjectManager(StubClientType *owner, Service::IServiceFactoryPtr serviceFactory)
                    : m_owner{owner}
                    , m_serviceFactory{serviceFactory}
                {
                }

            private:
                StubClientType *m_owner;

                Service::IServiceFactoryPtr m_serviceFactory;
                Common::UuidGenerator m_idGenerator;

                // IObjectManager
                virtual std::string CreateObject(std::string const &serviceId, std::string const &interfaceId) override final
                {
                    auto const instanceId = m_idGenerator.Generate();
                    using PSTuple = std::tuple<TProxyStubs<TSerializer> ... >;
                    auto stub = CreateStub<PSTuple>(interfaceId,
                        reinterpret_cast<std::integral_constant<std::size_t, std::tuple_size<PSTuple>::value> const *>(0));
                    auto instance = m_serviceFactory->Create(serviceId);
                                        {
                        LockGuard lock(m_owner->m_lock);
                        m_owner->m_stubs.insert(std::make_pair(instanceId, std::make_pair(std::move(stub), std::move(instance))));
                    }
                    return instanceId;
                }

                virtual void DestroyObject(std::string const &instanceId) override final
                {
                    StubService stubService;
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

                template <typename T, std::size_t I, typename = typename std::enable_if<I>::type>
                IStubPtr CreateStub(std::string const &interfaceId, std::integral_constant<std::size_t, I> const *)
                {
                    using PSType = typename std::tuple_element<I - 1, T>::type;
                    if (PSType::InterfaceId == interfaceId)
                        return std::make_shared<typename PSType::Stub>();
                    return CreateStub<T>(interfaceId,
                        reinterpret_cast<std::integral_constant<std::size_t, I - 1> const *>(0));
                }

                template <typename T>
                IStubPtr CreateStub(std::string const &interfaceId, std::integral_constant<std::size_t, 0> const *)
                {
                    throw std::runtime_error{"Stub for interface with id \"" + interfaceId + "\" not found."};
                }
            };

            friend class ObjectManager;
        };

    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_STUB_CLIENT_H__
