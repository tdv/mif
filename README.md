# MIF - C++11 MetaInfo Framework
MIF is a с++11 web-application framework designed for the backend micro-service development. The framework makes use of additional type meta-information.

# Version
1.1.0

# Features:
- libevent wrapper classes for the easy creation of simple web services
- Interface-based RPC marshalling with the callback support
- Reflection for c++ data structures with the inheritance support
- Json-serialization and the ability to use boost archives
- Database support (Implemented for PostgreSQL and SQLite)

# Compiler
The minimum compiler version required is gcc 5.3

# OS
Linux (Tested on Ubuntu 16.04 and CentOS 6.5, 7.2)

# Dependencies
- Boost
- zlib
- jsoncpp
- libevent
- libpq
- sqlite

# Build and install
```bash
git clone https://github.com/tdv/mif.git  
cd mif  
./download_third_party.sh  
mkdir build  
cd build  
cmake ..  
make  
make install  
```

You can try using CMAKE_INSTALL_PREFIX to select the installation directory

## Build examples
After mif is built, you can build examples
```bash
cd mif/examples/{sample_project}
mkdir build  
cd build  
cmake ..  
make  
```
## Run examples

Server:
```bash
cd mif/exammples/{sample_project}/bin  
./{sample_project}_server  
```
Client:
```bash
cd mif/exammples/{sample_project}/bin  
./{sample_project}_client  
```
Please use --help to get more information about runing an example

# Examples

## Hello World
[Source code](https://github.com/tdv/mif/tree/master/examples/hello_world)  
**Description**  
The "Hello World" example demonstrates a basic client-server application with the interface-based RPC marshaling and TCP communication with using boost.archives for data serialization

**Basic steps for building a client-server application with RPC**  
- define interface
- add meta-information about interface and methods
- create a data flow processing chain from a set of handler elements
- implement interface on the server side and create a server application
- create communication channel and proxy objects on the client side and create a client application  

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
This chain is common to all the examples. You can use it as-is in your projects. If you need to change the data exchange  protocol, change this chain.  

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

class Application
    : public Mif::Application::Application
{
public:
    Application(int argc, char const **argv)
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
    return Mif::Application::Run<Application>(argc, argv);
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

class Application
    : public Mif::Application::Application
{
public:
    Application(int argc, char const **argv)
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
    return Mif::Application::Run<Application>(argc, argv);
}
```

## Complex Type
[Source code](https://github.com/tdv/mif/tree/master/examples/complex_type)  
**Description**  
This example is the same as "Hello World". The difference is in calling remote methods with user-defined data structures as parameters and returning a value. The project structure is the same as in the previous project example, we only add the definition of user-defined data structures and meta-information.  

[User data structs](https://github.com/tdv/mif/blob/master/examples/complex_type/common/include/common/data/data.h)  
```cpp
// STD
#include <cstdint>
#include <map>
#include <string>

namespace Service
{
    namespace Data
    {

        using ID = std::string;

        struct Human
        {
            std::string name;
            std::string lastName;
            std::uint32_t age = 0;
        };

        struct Employee
            : public Human
        {
            std::string position;
        };

        using Employees = std::map<ID, Employee>;

    }   // namespace Data
}   // namespace Service
```

[Meta-information](https://github.com/tdv/mif/blob/master/examples/complex_type/common/include/common/meta/data.h)  
```cpp
// MIF
#include <mif/reflection/reflect_type.h>

// THIS
#include "common/data/data.h"

namespace Service
{
    namespace Data
    {
        namespace Meta
        {

            using namespace ::Service::Data;

            MIF_REFLECT_BEGIN(Human)
                MIF_REFLECT_FIELD(name)
                MIF_REFLECT_FIELD(lastName)
                MIF_REFLECT_FIELD(age)
            MIF_REFLECT_END()

            MIF_REFLECT_BEGIN(Employee, Human)
                MIF_REFLECT_FIELD(position)
            MIF_REFLECT_END()

        }   // namespace Meta
    }   // namespace Data
}   // namespace Service

MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Meta::Human)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Meta::Employee)
```

## Inheritance
[Source code](https://github.com/tdv/mif/tree/master/examples/inheritance)  
**Description**  
Compared to the previous examples this one adds the inteface inheritance. In the implementation you can query an interface which is out of hierarchy.

## Visitor
[Source code](https://github.com/tdv/mif/tree/master/examples/visitor)  
**Description**  
The "Visitor" example demonstrates the mechanism of remote callbacks for interface methods. This can be used as a starting point for publish / subscribe based applications.

## HTTP
[Source code](https://github.com/tdv/mif/tree/master/examples/http)  
**Description**  
The example demonstrates the work of the HTTP server with dual interface for processing raw HTTP requests and MIF RPC by HTTP.

## DB client
[Source code](https://github.com/tdv/mif/tree/master/examples/db_client)  
**Description**  
A classical work with databases.
