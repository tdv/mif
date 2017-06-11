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

# -------------------------------

# MIF - C++ MetaInfo Framework
Каркас для разработки веб-сервисов на C++.

# Текущее состояние
Version 1.0.0 - in progress ...

# Возможности
- Разработка http веб-сервисов
- Взаимодействие микросервисов (RPC c воможностью маршалинга объектов межде микросервисами)
- Сериализация структур данных на базе рефлексии
- Работа с БД (пока не реализовано)
- Поддержка нескольких языков программирования (пока не реализовано)

# Поддержка языков программирования
- C++ - основной язык для разработки микросервисов на базе MIF. (Стандарт: C++11)
- Python - планитруется поддеркжка для обращения к модулям, разработанным на C++

# Платформа
- Linux
- Windows (планируется поддержка в будущем)

# Компилятор
- gcc версии не ниже 5.4
- clang (планируется поддержка)

# Сборка
Сборка осуществляется с помощью утилит cmake + make

Так как MIF зависит от других библиотек, то в связи с этим возможно несколько вариантов сборки.
- Сборки MIF с предварительной загрузкой всех необходимые библиотеки.
- Сборка с подгрузкой всех библиотек с GitHub в момент выполнения cmake + make
- Сборка с подгрузкой всех библиотек с собственного сервера в момент выполнения cmake + make (пока не реализовано)
- Сборка с установленными в систему библиотеками

Пример сборки с предварительной загрузкой всех необходимых библиотек

git clone https://github.com/tdv/mif.git  
cd mif  
./download_third_party.sh  
mkdir build  
cd build  
cmake ..  
make  

Сборка примеров осуществляется после сборки MIF библиотек
Пример:

cd mif/examples/hello_world  
mkdir build  
cd build  
cmake ..  
make  

Собранный пример может быть запущен в разных консолях

Сервер:

./helloworld_server

Клиент:

./helloworld_client

Для получения описания всех параметров запуска примеров нужно запустить приложение с параметром --help

# Установка
make install

# Примеры
- hello_world - пример простого взаимодействия клиента с сервером
- complex_type - пример, демонстрирующий возможность передачи пользовательских структур данных межеду компонентами
- inheritance - пример, демонстрирующий наследование интерфейсов (построение иерархий) и запрос интерфейсов из имеющихся экземпляров.
- http - пример веб-сервиса, с возможностью формировать ответы в виде текста, в формате json и выдавать html-страницы, а так же есть RPC (работа с объектами, как в других примерах MIF).
- visitor - пример, демонстрирующий маршалинг интерфейсов между компонентами. Передача указателя в другой процесс / компонент и вызов методов объекта по переданному указателю из другого процесса. На базе этого примера можно разрабатывать сервисы с поддержкой publish/subscribe.
