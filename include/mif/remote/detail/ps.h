#ifndef __MIF_REMOTE_DETAIL_PS_H__
#define __MIF_REMOTE_DETAIL_PS_H__

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

            class ProxyStubException final
                : public std::runtime_error
            {
            public:
                using std::runtime_error::runtime_error;
            };

            template <typename TSerializer>
            class Proxy
            {
            public:
                Proxy(std::string const &instance)
                    : m_instance(instance)
                {
                }

                virtual ~Proxy() = default;

                virtual Common::Buffer Send(Common::Buffer && buffer) = 0;

                template <typename TResult, typename ... TParams>
                TResult RemoteCall(std::string const &interface, std::string const &method, TParams && ... params)
                {
                    try
                    {
                        using Serializer = typename TSerializer::Serializer;
                        using Deserializer = typename TSerializer::Deserializer;
                        Serializer serializer(m_instance, interface, method, true, std::forward<TParams>(params) ... );
                        auto response = Send(std::move(serializer.GetBuffer()));
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

                        return ResultExtractor<TResult>::Extract(deserializer);
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
            };

            template <typename TSerializer>
            struct IStub
            {
                using Serializer = typename TSerializer::Serializer;
                using Deserializer = typename TSerializer::Deserializer;

                virtual ~IStub() = default;
                virtual void Call(void *instance, Deserializer &request, Serializer &response) = 0;
            };

            template <typename TSerializer>
            class Stub
                : public IStub<TSerializer>
            {
            public:
                using BaseType = IStub<TSerializer>;
                using Serializer = typename BaseType::Serializer;
                using Deserializer = typename BaseType::Deserializer;

                virtual void Call(void *instance, Deserializer &request, Serializer &response) override final
                {
                    try
                    {
                        auto const &interfaceId = request.GetInterface();
                        if (interfaceId != GetInterfaceId())
                        {
                            throw ProxyStubException{"[Mif::Remote::Stub::Call] Bad interface id \"" +
                                interfaceId + "\" Needed \"" + GetInterfaceId() + "\""};
                        }
                        auto const &method = request.GetMethod();
                        InvokeMethod(instance, method, &request, &response);
                    }
                    catch (ProxyStubException const &)
                    {
                        throw;
                    }
                    catch (std::exception const &e)
                    {
                        throw ProxyStubException{"[Mif::Remote::Stub::Call] Failed to call method Error: " + std::string{e.what()}};
                    }
                }

            protected:
                virtual void InvokeMethod(void *, std::string const &method, void *, void *)
                {
                    throw ProxyStubException{"[Mif::Remote::Stub::InvokeMethod] Method \"" + method + "\" "
                        "of interface \"" + std::string{GetInterfaceId()} + "\" not found."};
                }

                template <typename TResult, typename TInterface, typename ... TParams>
                void InvokeRealMethod(TResult (*method)(TInterface &, std::tuple<TParams ... > && ),
                                      void *instance, void *deserializer, void *serializer)
                {
                    auto &inst = *reinterpret_cast<TInterface *>(instance);
                    auto params = reinterpret_cast<Deserializer *>(deserializer)->template GetParams<TParams ... >();
                    FunctionWrap<TResult>::Call(
                            [&method, &inst, &params] () { return method(inst, std::move(params)); },
                            *reinterpret_cast<Serializer *>(serializer)
                        );
                }

                virtual char const* GetInterfaceId() const
                {
                    return "[Mif::Remote::Stub::ProcessData] Default interface id.";
                }
            };

        }  // namespace Detail
    }   //  namespace Remote
}   // namespace Mif


#endif  // !__MIF_REMOTE_DETAIL_PS_H__
