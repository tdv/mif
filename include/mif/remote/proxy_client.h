#ifndef __MIF_REMOTE_PROXY_CLIENT_H__
#define __MIF_REMOTE_PROXY_CLIENT_H__

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
#include "mif/net/client.h"
#include "mif/remote/detail/iobject_manager_ps.h"

namespace Mif
{
    namespace Remote
    {

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

#endif  // !__MIF_REMOTE_PROXY_CLIENT_H__
