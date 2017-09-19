# MIF - C++11 MetaInfo Framework
MIF is a с++11 web-application framework designed for the backend micro-service development. The framework makes use of additional type meta-information.

**[ru] Post about MIF** [Microservices - MIF on C++![Build Status](https://habrahabr.ru/post/239571/)

**Note** [](https://github.com/tdv/mif/tree/master/examples/hello_world)  

# Version
1.2.0  

**NOTE:** The master branch is actively developed, use latest release for production use.

# Features:
- libevent wrapper classes for the easy creation of simple web services
- Interface-based RPC marshalling with the callback support
- Reflection for c++ data structures with the inheritance support
- Json, Xml serialization and the ability to use boost archives
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
**NOTE:** To develop your applications, you can use the application template. After downloading the MIF project, follow the steps
```bash
cd mif/template
mkdir build  
cd build  
cmake ..  
make  
```
After that you can change this template to create your own application.


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
- create a data flow processing chain from a set of handler elements or use one of the prepared applications templates
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

[Server interface implementation](https://github.com/tdv/mif/blob/master/examples/hello_world/server/src/hello_world.cpp)  
```cpp
...
// MIF
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
                ...

            private:
                ...

                // IHelloWorld
                virtual void AddWord(std::string const &word) override final
                {
                    ...
                }
                virtual std::string GetText() const override final
                {
                    ...
                }
                virtual void Clean() override final
                {
                    ...
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
#include <mif/application/tcp_service.h>

// COMMON
#include "common/id/service.h"
#include "common/ps/ihello_world.h"

class Application
    : public Mif::Application::TcpService
{
public:
    using TcpService::TcpService;

private:
    // Mif.Application.TcpService
    virtual void Init(Mif::Service::FactoryPtr factory) override final
    {
        factory->AddClass<Service::Id::HelloWorld>();
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
#include <mif/application/tcp_service_client.h>
#include <mif/common/log.h>

// COMMON
#include "common/ps/ihello_world.h"

class Application
    : public Mif::Application::TcpServiceClient
{
public:
    using TcpServiceClient::TcpServiceClient;

private:
    // Mif.Application.TcpServiceClient
    virtual void Init(Mif::Service::IFactoryPtr factory) override final
    {
        auto service = factory->Create<Service::IHelloWorld>("HelloWorld");

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

## HTTP Echo Server
[Source code](https://github.com/tdv/mif/tree/master/examples/http_echo_server)  
**Description**  
The example demonstrates the work of the simple HTTP echo server.
[HTTP-echo-server](https://github.com/tdv/mif/blob/master/examples/http_echo_server/src/main.cpp)  
```cpp
// MIF
#include <mif/application/http_server.h>
#include <mif/common/log.h>
#include <mif/net/http/constants.h>

class Application
    : public Mif::Application::HttpServer
{
public:
    using HttpServer::HttpServer;

private:
    // Mif.Application.HttpServer
    virtual void Init(Mif::Net::Http::ServerHandlers &handlers) override final
    {
        handlers["/"] = [] (Mif::Net::Http::IInputPack const &request,
                Mif::Net::Http::IOutputPack &response)
        {
            auto data = request.GetData();

            MIF_LOG(Info) << "Process request \"" << request.GetPath() << request.GetQuery() << "\"\t Data: "
                    << (data.empty() ? std::string{"null"} : std::string{std::begin(data), std::end(data)});

            response.SetCode(Mif::Net::Http::Code::Ok);
            response.SetHeader(Mif::Net::Http::Constants::Header::Connection::GetString(),
                               Mif::Net::Http::Constants::Value::Connection::Close::GetString());

            response.SetData(std::move(data));
        };
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
```
**Test**
```bash
curl -iv -X POST "http://localhost:55555/" -d 'Test data'
```

## HTTP CRUD
[Source code](https://github.com/tdv/mif/tree/master/examples/http_crud)  
**Description**  
The example demonstrates the Json API (CRUD operations) on HTTP server.


**Test**
- run PostgreSQL
- apply DB schema from folder db
- modify config/config.xml
- run http_crud server with parameter --config (in folder 'bin' execute ./http_crud --config=../config/config.xml)
- execute commands
```bash
curl -i -X POST "http://localhost:55555/employee/create" -d '{"name":"Ivan", "lastName":"Ivanov","age":33,"email":"ivanov@mycompany.com","position":"Developer","rate":200000.00}'

curl -i "http://localhost:55555/employee/read?id=1"

curl -i "http://localhost:55555/employee/update?id=1" -d '{"name":"Ivan", "lastName":"Ivanov","age":33,"email":"ivanov@mycompany.com","position":"Developer","rate":220000.00}'

curl -i "http://localhost:55555/employee/list?limit=2&offset=0"

curl -i "http://localhost:55555/employee/delete?id=1"
```

## Microservices
[Source code](https://github.com/tdv/mif/tree/master/examples/microservices)  
**Description**  
The example demonstrates communication between two microservices (the example is more powerful version of http crud).
**Test**
- run PostgreSQL
- apply DB schema from folder db
- modify config/storage.xml
- run components
```bash
./storage —config=storage.xml
./service --config=service.xml
```
- execute commands
```bash
curl -i -X POST "http://localhost:55555/employee/create" -d '{"name":"Ivan", "lastName":"Ivanov","age":33,"email":"ivanov@mycompany.com","position":"Developer","rate":200000.00}'

curl -i "http://localhost:55555/employee/read?id=1"

curl -i "http://localhost:55555/employee/update?id=1" -d '{"name":"Ivan", "lastName":"Ivanov","age":33,"email":"ivanov@mycompany.com","position":"Developer","rate":220000.00}'

curl -i "http://localhost:55555/employee/list?limit=2&offset=0"

curl -i "http://localhost:55555/employee/delete?id=1"
```

## Reflection
[Source code](https://github.com/tdv/mif/tree/master/examples/reflection)  
**Description**  
The example demonstrates the mechanism of C++ data struct reflection. This can be used as a starting point for building application with serialization, ORM and REST API.

## DB client
[Source code](https://github.com/tdv/mif/tree/master/examples/db_client)  
**Description**  
A classical work with databases.
