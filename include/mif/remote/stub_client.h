#ifndef __MIF_REMOTE_STUB_CLIENT_H__
#define __MIF_REMOTE_STUB_CLIENT_H__

// STD
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>

// MIF
#include "mif/net/client.h"
#include "mif/remote/detail/iobject_manager_ps.h"

namespace Mif
{
    namespace Remote
    {

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

            ~StubClient() = default;

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
                return "new_instance_" + classId;
            }

            virtual void DestroyObject(std::string const &instanceId) override final
            {
                (void)instanceId;
            }
        };

    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_STUB_CLIENT_H__
