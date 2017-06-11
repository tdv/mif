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
**Description**  
The "Hello World" sample demonstrate basic client-server application with rpc based on interface marshaling and communicate by TCP with using boost.archives for data serialization


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
