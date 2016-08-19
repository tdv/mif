#ifndef __MIF_REMOTE_PROXY_STUB_H__
#define __MIF_REMOTE_PROXY_STUB_H__

// STD
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace Mif
{
    namespace Remote
    {

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

        public:
            template <typename TResult, typename ... TParams>
            TResult RemoteCall(std::string const &interface, std::string const &method, TParams && ... params)
            {
                try
                {
                    using Serializer = typename TSerializer::Serializer;
                    Serializer serializer(m_instance, interface, method, std::forward<TParams>(params) ... );
                    auto response = m_transport.Send(std::move(serializer.GetBuffer()));
                    throw ProxyStubException{method + " not implemented."};
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
            virtual void InvokeMethod(std::string const &method, void *)
            {
                throw ProxyStubException{"[Mif::Remote::Stub::InvokeMethod] Method \"" + method + "\" "
                    "of interface \"" + std::string{GetInterfaceId()} + "\" not found."};
            }

            using Deserializer = typename TSerializer::Deserializer;

            template <typename TResult, typename TInterface, typename ... TParams>
            void InvokeRealMethod(TResult (*method)(TInterface &, std::tuple<TParams ... > && ), void *params)
            {
                method(*reinterpret_cast<TInterface *>(m_impl->GetInstance()),
                    reinterpret_cast<Deserializer *>(params)->template GetParams<TParams ... >());
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
                using Buffer = typename TTransport::Buffer;
                using DataHandler = typename TTransport::DataHandler;

                Stub<TSerializer, TTransport> &m_owner;
                std::string m_instanceId;
                InstancePtr<TInterface> m_instance;
                TTransport m_transport;

                virtual void Init() override
                {
                    m_transport.SetHandler(std::bind(&Impl::OnData, this->shared_from_this(), std::placeholders::_1));
                }

                virtual void Done() override
                {
                    m_transport.SetHandler(DataHandler{});
                }

                virtual void* GetInstance() override
                {
                    return m_instance.get();
                }

                void OnData(Buffer && buffer)
                {
                    try
                    {
                        if (buffer.empty())
                            throw ProxyStubException{"[Mif::Remote::Stub::OnData] Empty data."};
                        Deserializer deserializer(std::move(buffer));
                        if (!deserializer.IsRequest())
                            throw ProxyStubException{"[Mif::Remote::Stub::OnData] Bad request type \"" + deserializer.GetType() + "\""};
                        if (deserializer.GetInstance() != m_instanceId)
                        {
                            throw ProxyStubException{"[Mif::Remote::Stub::OnData] Bad instance id \"" + deserializer.GetInstance() + "\" "
                                "Needed instance id \"" + m_instanceId + "\""};
                        }
                        if (deserializer.GetInterface() != m_owner.GetInterfaceId())
                        {
                            throw ProxyStubException{"[Mif::Remote::Stub::OnData] Bad interface for instance whith id \"" +
                                deserializer.GetInterface() + "\" Needed \"" + m_owner.GetInterfaceId() + "\""};
                        }
                        auto const &method = deserializer.GetMethod();
                        if (method.empty())
                        {
                            throw ProxyStubException{"[Mif::Remote::Stub::OnData] Empty method name of interface \"" +
                                std::string{m_owner.GetInterfaceId()} + "\" for instance with id \"" + m_instanceId + "\""};
                        }
                        m_owner.InvokeMethod(method, &deserializer);
                    }
                    catch (ProxyStubException const &)
                    {
                        throw;
                    }
                    catch (std::exception const &e)
                    {
                        throw ProxyStubException{"[Mif::Remote::Stub::OnData] Failed to call method for instance with id \"" +
                                                m_instanceId + "\" Error: " + std::string{e.what()}};
                    }
                }
            };

            template <typename TInterface>
            friend class Impl;

            std::shared_ptr<IImpl> m_impl;

            virtual char const* GetInterfaceId() const
            {
                return "[Mif::Remote::Stub::OnData] Default interface id.";
            }
        };

    }   //  namespace Remote
}   // namespace Mif


#endif  // !__MIF_REMOTE_PROXY_STUB_H__
