#ifndef __MIF_REMOTE_PROXY_STUB_H__
#define __MIF_REMOTE_PROXY_STUB_H__

// STD
#include <stdexcept>
#include <string>
#include <utility>

namespace Mif
{
    namespace Remote
    {

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
                    throw std::runtime_error{method + " not implemented."};
                }
                catch (std::exception const &e)
                {
                    throw std::runtime_error{"[Mif::Remote::Proxy::RemoteCall] Failed to call remote method \"" +
                        interface + "::" + method + "\" for instance with id \"" + m_instance + "\". Error: " +
                        std::string{e.what()}};
                }
            }

        private:
            std::string m_instance;
            TTransport m_transport;
        };

    }   //  namespace Remote
}   // namespace Mif


#endif  // !__MIF_REMOTE_PROXY_STUB_H__
