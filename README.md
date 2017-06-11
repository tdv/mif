# MIF - C++11 MetaInfo Framework
MIF is a framework on modern C++11 for building web-application backend based on micro-services with using additional meta-information about types

# Version
1.0.0

# Features:
- The wrappers classes over the libevent library for easy creation of simple web services
- Rpc based on the interfaces marshalling with support for callback
- Reflection for c++ data structures with inheritance supporting
- Serialization in json and the ability to use boost archives
- Support working with databases (Implemented for PostgreSQL and SQLite)

# Compiler
The minimum compiler version required is gcc 5.3

# OS
Linux (Tested on Ubuntu 16.04 and CentOS 6.5, 7.2)

# Build and install
git clone https://github.com/tdv/mif.git  
cd mif  
./download_third_party.sh  
mkdir build  
cd build  
cmake ..  
make  
make install  

You can try using CMAKE_INSTALL_PREFIX to select the installation directory

## Build examples
After mif builds, you can build samples
cd mif/examples/{sample_project}
mkdir build  
cd build  
cmake ..  
make  

## Run examples

Server:
cd mif/exammples/{sample_project}/bin  
./{sample_project}_server  

Client:
cd mif/exammples/{sample_project}/bin  
./{sample_project}_client  

Please use --help for get more information about run a sample

# Examples

## Hello World
[Source code](https://github.com/tdv/mif/tree/master/examples/hello_world)  
**Description**  
The "Hello World" sample demonstrate basic client-server application with rpc based on interface marshaling and communicate by TCP with using boost.archives for data serialization
**Basic steps for build client-server application with rpc**  
- define interface
- add a meta-information about interface and methods
- create a data flow processing chain from a set of handler elements
- implement interface on server side and create server application
- create communication channel and proxy objects on client side and create client application  

[Common interface](https://github.com/tdv/mif/blob/master/examples/hello_world/common/include/common/interface/ihello_world.h)  
```cpp
// STD
#include <string>

// MIF
#include <mif/service/iservice.h>

namespace Service
{

    struct IHelloWorld
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
        virtual void AddWord(std::string const &word) = 0;
        virtual std::string GetText() const = 0;
        virtual void Clean() = 0;
    };

}   // namespace Service
```

[Common interface meta-information](https://github.com/tdv/mif/blob/master/examples/hello_world/common/include/common/ps/ihello_world.h)  
```cpp
// STD
#include <string>

// MIF
#include <mif/remote/ps.h>

// THIS
#include "common/interface/ihello_world.h"

namespace Service
{
    namespace Meta
    {

        using namespace ::Service;

        MIF_REMOTE_PS_BEGIN(IHelloWorld)
            MIF_REMOTE_METHOD(AddWord)
            MIF_REMOTE_METHOD(GetText)
            MIF_REMOTE_METHOD(Clean)
        MIF_REMOTE_PS_END()

    }   // namespace Meta
}   // namespace Service

MIF_REMOTE_REGISTER_PS(Service::Meta::IHelloWorld)
```

[Common data processing chain](https://github.com/tdv/mif/blob/master/examples/common/include/common/client.h)  
```cpp
// STD
#include <chrono>
#include <cstdint>
#include <memory>

// BOOST
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

// MIF
#include <mif/common/thread_pool.h>
#include <mif/net/clients_chain.h>
#include <mif/net/client_factory.h>
#include <mif/net/clients/parallel_handler.h>
#include <mif/net/clients/frame_reader.h>
#include <mif/net/clients/frame_writer.h>
#include <mif/net/clients/gzip_compressor.h>
#include <mif/net/clients/gzip_decompressor.h>
#include <mif/remote/ps_client.h>
#include <mif/remote/serialization/serialization.h>
#include <mif/remote/serialization/boost.h>
#include <mif/remote/serialization/json.h>
#include <mif/service/ifactory.h>
#include <mif/service/factory.h>
#include <mif/service/make.h>

namespace Service
{
    namespace Ipc
    {
        namespace Detail
        {

            namespace Boost
            {

                using Serializer = Mif::Remote::Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
                using Deserializer = Mif::Remote::Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
                using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<Serializer, Deserializer>;

            }   // namespace Boost

            namespace Json
            {

                using Serializer = Mif::Remote::Serialization::Json::Serializer;
                using Deserializer = Mif::Remote::Serialization::Json::Deserializer;
                using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<Serializer, Deserializer>;

            }   // namespace Json

        }   // namespace Detail

        using PSClient = Mif::Remote::PSClient<Detail::Boost::SerializerTraits>;
        using WebPSClient = Mif::Remote::PSClient<Detail::Json::SerializerTraits>;

        namespace Detail
        {

            using ProtocolChain = Mif::Net::ClientsChain
                <
                    Mif::Net::Clients::FrameReader,
                    Mif::Net::Clients::ParallelHandler,
                    Mif::Net::Clients::GZipDecompressor,
                    PSClient,
                    Mif::Net::Clients::GZipCompressor,
                    Mif::Net::Clients::FrameWriter
                >;

            using ClientFactory = Mif::Net::ClientFactory<ProtocolChain>;

            using WebProtocolChain = Mif::Net::ClientsChain
                <
                    WebPSClient
                >;

            using WebClientFactory = Mif::Net::ClientFactory<WebProtocolChain>;

        }   // namespace Detail

        using ClientsChain = Detail::ProtocolChain;
        using WebClientsChain = Detail::WebProtocolChain;

        inline Mif::Net::IClientFactoryPtr MakeClientFactory(std::uint16_t threadCount,
                std::chrono::microseconds const &timeout,
                Mif::Service::IFactoryPtr factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::IFactory>())
        {
            auto workers = Mif::Common::CreateThreadPool(threadCount);

            return std::make_shared<Detail::ClientFactory>
                (
                    Mif::Common::MakeCreator<Mif::Net::Clients::ParallelHandler>(workers),
                    Mif::Common::MakeCreator<PSClient>(timeout, factory)
                );
        }

        inline Mif::Net::IClientFactoryPtr MakeWebClientFactory(std::chrono::microseconds const &timeout,
                Mif::Service::IFactoryPtr factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::IFactory>())
        {
            return std::make_shared<Detail::WebClientFactory>
                (
                    Mif::Common::MakeCreator<WebPSClient>(timeout, factory)
                );
        }

    }   // namespace Ipc
}   // namespace Service
```

It may seem that this is the most difficult part of the system. But do not be afraid. This is not true.  
This chain is common to all examples. You can use it as-is in your projects. If you need to change the data exchange  protocol, change this chain.  

[Server interface implementation](https://github.com/tdv/mif/blob/master/examples/hello_world/server/src/hello_world.cpp)  
```cpp// STD
#include <list>
#include <mutex>

// MIF
#include <mif/common/log.h>
#include <mif/service/creator.h>

// THIS
#include "common/id/service.h"
#include "common/interface/ihello_world.h"

namespace Service
{
    namespace Detail
    {
        namespace
        {

            class HelloWorld
                : public IHelloWorld
            {
            public:
                HelloWorld()
                {
                    MIF_LOG(Info) << "HelloWorld";
                }

                ~HelloWorld()
                {
                    MIF_LOG(Info) << "~HelloWorld";
                }

            private:
                using LockType = std::mutex;
                using LockGuard = std::lock_guard<LockType>;

                mutable LockType m_lock;
                std::list<std::string> m_words;

                // IHelloWorld
                virtual void AddWord(std::string const &word) override final
                {
                    MIF_LOG(Info) << "AddWord: " << word;
                    LockGuard lock(m_lock);
                    m_words.push_back(word);
                }

                virtual std::string GetText() const override final
                {
                    std::string text;
                    {
                        LockGuard lock(m_lock);
                        MIF_LOG(Info) << "GetText. Creating text in " << m_words.size() << " words.";
                        for (auto const &word : m_words)
                        {
                            if (!text.empty())
                                text += " ";
                            text += word;
                        }
                        MIF_LOG(Info) << "GetText. Created text in " << m_words.size() << " words. Text \"" << text << "\"";
                    }
                    return text;
                }


                virtual void Clean() override final
                {
                    LockGuard lock(m_lock);
                    MIF_LOG(Info) << "Clean. Remove " << m_words.size() << " words.";
                    decltype(m_words){}.swap(m_words);
                }

            };

        }   // namespace
    }   // namespace Detail
}   // namespace Service

MIF_SERVICE_CREATOR
(
    ::Service::Id::HelloWorld,
    ::Service::Detail::HelloWorld
)
```

[Server application](https://github.com/tdv/mif/blob/master/examples/hello_world/server/src/main.cpp)  
```cpp
// MIF
#include <mif/application/application.h>
#include <mif/common/log.h>
#include <mif/net/tcp_server.h>

// COMMON
#include "common/client.h"
#include "common/ps/ihello_world.h"

// THIS
#include "common/id/service.h"

class Applicatin
    : public Mif::Application::Application
{
public:
    Applicatin(int argc, char const **argv)
        : Mif::Application::Application{argc, argv}
    {
        boost::program_options::options_description options{"Server options"};
        options.add_options()
                ("host", boost::program_options::value<std::string>()->default_value("0.0.0.0"), "Server host")
                ("port", boost::program_options::value<std::string>()->default_value("55555"), "Server port")
                ("workers", boost::program_options::value<std::uint16_t>()->default_value(8), "Workers thread count");

        AddCustomOptions(options);
    }

private:
    std::unique_ptr<Mif::Net::TCPServer> m_server;

    // Mif.Application.Application
    virtual void OnStart() override final
    {
        auto const &options = GetOptions();

        auto const host = options["host"].as<std::string>();
        auto const port = options["port"].as<std::string>();
        auto const workers = options["workers"].as<std::uint16_t>();

        MIF_LOG(Info) << "Starting server on " << host << ":" << port;

        auto factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::Factory>();
        factory->AddClass<Service::Id::HelloWorld>();

        std::chrono::microseconds const timeout{10 * 1000 * 1000};

        auto clientFactory = Service::Ipc::MakeClientFactory(workers, timeout, factory);

        m_server.reset(new Mif::Net::TCPServer{host, port, clientFactory});

        MIF_LOG(Info) << "Server is successfully started.";
    }

    virtual void OnStop() override final
    {
        MIF_LOG(Info) << "Stopping server ...";

        m_server.reset();

        MIF_LOG(Info) << "Server is successfully stopped.";
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Applicatin>(argc, argv);
}
```

[Client application](https://github.com/tdv/mif/blob/master/examples/hello_world/client/src/main.cpp)  
```cpp
// MIF
#include <mif/application/application.h>
#include <mif/common/log.h>
#include <mif/net/tcp_clients.h>

// COMMON
#include "common/client.h"
#include "common/ps/ihello_world.h"

class Applicatin
    : public Mif::Application::Application
{
public:
    Applicatin(int argc, char const **argv)
        : Mif::Application::Application{argc, argv}
    {
        boost::program_options::options_description options{"Client options"};
        options.add_options()
                ("host", boost::program_options::value<std::string>()->default_value("0.0.0.0"), "Server host")
                ("port", boost::program_options::value<std::string>()->default_value("55555"), "Server port");

        AddCustomOptions(options);
    }

private:
    // Mif.Application.Application
    virtual void OnStart() override final
    {
        auto const &options = GetOptions();

        auto const host = options["host"].as<std::string>();
        auto const port = options["port"].as<std::string>();

        MIF_LOG(Info) << "Starting client on " << host << ":" << port;

        std::chrono::microseconds timeout{10 * 1000 * 1000};

        auto clientFactory = Service::Ipc::MakeClientFactory(4, timeout);

        Mif::Net::TCPClients clients(clientFactory);

        auto proxy = std::static_pointer_cast<Service::Ipc::ClientsChain>(clients.RunClient(host, port));

        MIF_LOG(Info) << "Client is successfully started.";

        auto client = proxy->GetClientItem<Service::Ipc::PSClient>();

        auto service = client->CreateService<Service::IHelloWorld>("HelloWorld");

        MIF_LOG(Info) << "Client started.";

        MIF_LOG(Info) << "Add words.";

        service->AddWord("Hello");
        service->AddWord("World");
        service->AddWord("!!!");

        MIF_LOG(Info) << "Result from server: \"" << service->GetText() << "\"";

        MIF_LOG(Info) << "Clean.";
        service->Clean();

        MIF_LOG(Info) << "Result from server: \"" << service->GetText() << "\"";
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Applicatin>(argc, argv);
}
```

## Cmplex Type
[Source code](https://github.com/tdv/mif/tree/master/examples/complex_type)  
**Description**  
TODO  

## Inheritance
[Source code](https://github.com/tdv/mif/tree/master/examples/inheritance)  
**Description**  
TODO  

## Visitor
[Source code](https://github.com/tdv/mif/tree/master/examples/visitor)  
**Description**  
TODO  

## Http
[Source code](https://github.com/tdv/mif/tree/master/examples/http)  
**Description**  
TODO  

## Http
[Source code](https://github.com/tdv/mif/tree/master/examples/db_client)  
**Description**  
TODO  
