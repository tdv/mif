#ifndef __MIF_REMOTE_PROXY_STUB_H__
#define __MIF_REMOTE_PROXY_STUB_H__

// STD
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

// MIF
#include "mif/common/types.h"

namespace Mif
{
    namespace Remote
    {
        namespace Detail
        {

            template <typename TResult>
            struct FunctionWrap
            {
                template <typename TFunc, typename TStorage>
                static void Call(TFunc func, TStorage &storage)
                {
                    auto res = func();
                    storage.PutParams(std::move(res));
                }
            };

            template <>
            struct FunctionWrap<void>
            {
                template <typename TFunc, typename TStorage>
                static void Call(TFunc func, TStorage &storage)
                {
                    func();
                }
            };

            template <typename TResult>
            struct ResultExtractor
            {
                template <typename TStorage>
                static TResult Extract(TStorage &storage)
                {
                    return std::get<0>(storage.template GetParams<TResult>());
                }
            };

            template <>
            struct ResultExtractor<void>
            {
                template <typename TStorage>
                static void Extract(TStorage &)
                {
                }
            };

        }  // namespace Detail

        class ProxyStubException final
            : public std::runtime_error
        {
        public:
            using std::runtime_error::runtime_error;
        };

        template <typename TSerializer, typename TTransport>
        class Proxy
        {
        public:
            Proxy(std::string const &instance, TTransport transport)
                : m_instance(instance)
                , m_transport(std::move(transport))
            {
            }

            virtual ~Proxy() = default;

            template <typename TResult, typename ... TParams>
            TResult RemoteCall(std::string const &interface, std::string const &method, TParams && ... params)
            {
                try
                {
                    using Serializer = typename TSerializer::Serializer;
                    using Deserializer = typename TSerializer::Deserializer;
                    Serializer serializer(m_instance, interface, method, true, std::forward<TParams>(params) ... );
                    auto response = m_transport.Send(std::move(serializer.GetBuffer()));
                    Deserializer deserializer(std::move(response));
                    if (!deserializer.IsResponse())
                        throw ProxyStubException{"[Mif::Remote::Proxy::RemoteCall] Bad response type \"" + deserializer.GetType() + "\""};
                    auto const &instance = deserializer.GetInstance();
                    if (instance != m_instance)
                    {
                        throw ProxyStubException{"[Mif::Remote::Proxy::RemoteCall] Bad instance id \"" + instance + "\" "
                            "Needed instance id \"" + m_instance + "\""};
                    }
                    auto const &interfaceId = deserializer.GetInterface();
                    if (interface != interfaceId)
                    {
                        throw ProxyStubException{"[Mif::Remote::Proxy::RemoteCall] Bad interface for instance whith id \"" +
                            interfaceId + "\" Needed \"" + interface + "\""};
                    }
                    auto const &methodId = deserializer.GetMethod();
                    if (method != methodId)
                    {
                        throw ProxyStubException{"[Mif::Remote::Proxy::RemoteCall] Method \"" + methodId + "\" "
                            "of interface \"" + interface + "\" for instance with id \"" + m_instance + "\" "
                            "not found. Needed method \"" + method + "\""};
                    }

                    return Detail::ResultExtractor<TResult>::Extract(deserializer);
                }
                catch (std::exception const &e)
                {
                    throw ProxyStubException{"[Mif::Remote::Proxy::RemoteCall] Failed to call remote method \"" +
                        interface + "::" + method + "\" for instance with id \"" + m_instance + "\". Error: " +
                        std::string{e.what()}};
                }
            }

        private:
            std::string m_instance;
            TTransport m_transport;
        };

        template <typename TSerializer, typename TTransport>
        class Stub
        {
        public:
            template <typename T>
            using InstancePtr = std::shared_ptr<T>;

            template <typename TInterface>
            Stub(std::string const &instanceId, InstancePtr<TInterface> instance, TTransport && transport)
                : m_impl(std::make_shared<Impl<TInterface>>(*this, instanceId, instance, std::move(transport)))
            {
            }

            virtual ~Stub() = default;

            void Init()
            {
                m_impl->Init();
            }
            void Done()
            {
                m_impl->Dode();
            }

        protected:
            virtual void InvokeMethod(std::string const &method, void *, void *)
            {
                throw ProxyStubException{"[Mif::Remote::Stub::InvokeMethod] Method \"" + method + "\" "
                    "of interface \"" + std::string{GetInterfaceId()} + "\" not found."};
            }

            using Serializer = typename TSerializer::Serializer;
            using Deserializer = typename TSerializer::Deserializer;

            template <typename TResult, typename TInterface, typename ... TParams>
            void InvokeRealMethod(TResult (*method)(TInterface &, std::tuple<TParams ... > && ),
                                  void *deserializer, void *serializer)
            {
                auto &instance = *reinterpret_cast<TInterface *>(m_impl->GetInstance());
                auto params = reinterpret_cast<Deserializer *>(deserializer)->template GetParams<TParams ... >();
                Detail::FunctionWrap<TResult>::Call(
                        [&method, &instance, &params] () { return method(instance, std::move(params)); },
                        *reinterpret_cast<Serializer *>(serializer)
                    );
            }

        private:
            struct IImpl
            {
                virtual ~IImpl() = default;
                virtual void Init() = 0;
                virtual void Done() = 0;
                virtual void* GetInstance() = 0;
            };

            template <typename TInterface>
            class Impl final
                : public IImpl
                , public std::enable_shared_from_this<Impl<TInterface>>
            {
            public:
                Impl(Stub<TSerializer, TTransport> &owner, std::string const &instanceId,
                     InstancePtr<TInterface> instance, TTransport && transport)
                    : m_owner(owner)
                    , m_instanceId(instanceId)
                    , m_instance(instance)
                    , m_transport(std::move(transport))
                {
                }

            private:
                using DataHandler = typename TTransport::DataHandler;

                Stub<TSerializer, TTransport> &m_owner;
                std::string m_instanceId;
                InstancePtr<TInterface> m_instance;
                TTransport m_transport;

                virtual void Init() override
                {
                    m_transport.SetHandler(std::bind(&Impl::ProcessData, this->shared_from_this(), std::placeholders::_1));
                }

                virtual void Done() override
                {
                    m_transport.SetHandler(DataHandler{});
                }

                virtual void* GetInstance() override
                {
                    return m_instance.get();
                }

                Common::Buffer ProcessData(Common::Buffer && buffer)
                {
                    try
                    {
                        if (!buffer.first)
                            throw ProxyStubException{"[Mif::Remote::Stub::ProcessData] Empty data."};
                        Deserializer deserializer(std::move(buffer));
                        if (!deserializer.IsRequest())
                            throw ProxyStubException{"[Mif::Remote::Stub::ProcessData] Bad request type \"" + deserializer.GetType() + "\""};
                        auto const &instanceId = deserializer.GetInstance();
                        if (instanceId != m_instanceId)
                        {
                            throw ProxyStubException{"[Mif::Remote::Stub::ProcessData] Bad instance id \"" + instanceId + "\" "
                                "Needed instance id \"" + m_instanceId + "\""};
                        }
                        auto const &interfaceId = deserializer.GetInterface();
                        if (interfaceId != m_owner.GetInterfaceId())
                        {
                            throw ProxyStubException{"[Mif::Remote::Stub::ProcessData] Bad interface for instance whith id \"" +
                                interfaceId + "\" Needed \"" + m_owner.GetInterfaceId() + "\""};
                        }
                        auto const &method = deserializer.GetMethod();
                        if (method.empty())
                        {
                            throw ProxyStubException{"[Mif::Remote::Stub::ProcessData] Empty method name of interface \"" +
                                std::string{m_owner.GetInterfaceId()} + "\" for instance with id \"" + m_instanceId + "\""};
                        }
                        Serializer serializer(instanceId, interfaceId, method, false);
                        m_owner.InvokeMethod(method, &deserializer, &serializer);
                        return serializer.GetBuffer();
                    }
                    catch (ProxyStubException const &)
                    {
                        throw;
                    }
                    catch (std::exception const &e)
                    {
                        throw ProxyStubException{"[Mif::Remote::Stub::ProcessData] Failed to call method for instance with id \"" +
                                                m_instanceId + "\" Error: " + std::string{e.what()}};
                    }
                }
            };

            template <typename TInterface>
            friend class Impl;

            std::shared_ptr<IImpl> m_impl;

            virtual char const* GetInterfaceId() const
            {
                return "[Mif::Remote::Stub::ProcessData] Default interface id.";
            }
        };

    }   //  namespace Remote
}   // namespace Mif


#endif  // !__MIF_REMOTE_PROXY_STUB_H__
