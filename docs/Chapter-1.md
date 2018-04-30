# The thivial web-application

In this chapter, we will consider a trivial web-application. The chapter will be the basis for a easy going  understanding of the process of creating your own web-applications. I'll try to build two simple applications:
- simple web service that works like a cache
- simple http echo-server

To create your own MIF-based application, you can try using a template. The application template supplied with the framework. To create your own application from the MIF template, you can do the following:
- clone the MIF framework
- copy the template folder to the desired location
- open and edit the cmake build script
- make the build directory
- enter code of the web service
- build application
- run and test it
- enjoy your work :)

```bash
git clone https://github.com/tdv/mif.git
cp -r mif/template/ ./trivial_web_service
cd trivial_web_service
mkdir build
```
Edit the build script:
- open CMakeLists.txt and find 'project(my_app)'
- change the project name to 'trivial_web_service'
- save changes

In the source file src/main.cpp you can see a simple application. This application is a base that can:
- show version information and help
- run in daemon mode

But I promised to create a web service.  
Well. Open the source file src/main.cpp and enter the following code:
```cpp
// STD
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>

// MIF
#include <mif/application/http_server.h>
#include <mif/common/static_string.h>
#include <mif/common/types.h>
#include <mif/common/unused.h>
#include <mif/net/http/constants.h>

class Application
    : public Mif::Application::HttpServer
{
public:
    using HttpServer::HttpServer;

private:
    using KeyParamId = MIF_STATIC_STR("key");

    using LockType = std::mutex;
    using LockGuard = std::lock_guard<LockType>;
    using Cache = std::map<std::string, Mif::Common::Buffer>;

    LockType m_lock;
    Cache m_cache;

    std::string GetKey(Mif::Net::Http::IInputPack::Params const &params) const
    {
        auto const key = params.find(KeyParamId::Value);
        if (key == std::end(params))
            throw std::invalid_argument{"No key"};
        return key->second;
    }

    virtual void Init(Mif::Net::Http::ServerHandlers &handlers) override final
    {
        handlers["/set"] = std::bind(&Application::Set, this, std::placeholders::_1, std::placeholders::_2);
        handlers["/get"] = std::bind(&Application::Get, this, std::placeholders::_1, std::placeholders::_2);
        handlers["/del"] = std::bind(&Application::Del, this, std::placeholders::_1, std::placeholders::_2);
        handlers["/clean"] = std::bind(&Application::Clean, this, std::placeholders::_1, std::placeholders::_2);
    }

    void Set(Mif::Net::Http::IInputPack const &req, Mif::Net::Http::IOutputPack &resp)
    {
        Mif::Common::Unused(resp);

        auto const params = req.GetParams();
        auto const key = GetKey(params);

        auto data = req.GetData();

        LockGuard lock{m_lock};
        m_cache[key] = std::move(data);
    }

    void Get(Mif::Net::Http::IInputPack const &req, Mif::Net::Http::IOutputPack &resp)
    {
        auto const params = req.GetParams();
        auto const key = GetKey(params);

        {
            LockGuard lock{m_lock};
            auto const iter = m_cache.find(key);
            if (iter == std::end(m_cache))
                throw std::invalid_argument{"Key \"" + key + "\" not found."};
            resp.SetData(iter->second);
        }

        resp.SetHeader(Mif::Net::Http::Constants::Header::Response::ContentType::Value,
                "text/html; charset=UTF-8");
    }

    void Del(Mif::Net::Http::IInputPack const &req, Mif::Net::Http::IOutputPack &resp)
    {
        Mif::Common::Unused(resp);

        auto const params = req.GetParams();
        auto const key = GetKey(params);

        LockGuard lock{m_lock};
        auto const iter = m_cache.find(key);
        if (iter == std::end(m_cache))
            throw std::invalid_argument{"Key \"" + key + "\" not found."};
        m_cache.erase(iter);
    }

    void Clean(Mif::Net::Http::IInputPack const &req, Mif::Net::Http::IOutputPack &resp)
    {
        Mif::Common::Unused(req, resp);

        LockGuard lock{m_lock};
        m_cache.clear();
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
```

Now you can build, run and test your own first REST-like web application
```bash
cd build
cmake ..
make
../bin/trivial_web_service
```
The service has an API to get / set / delete a value and clear all values.  
Tests:
```bash
curl -is "http://localhost:55555/set?key=my_test_key" -d 'My any test string value.'
HTTP/1.1 200 OK
Date: Mon, 09 Apr 2018 21:44:34 GMT
Content-Length: 0
Content-Type: text/html; charset=ISO-8859-1

curl -is "http://localhost:55555/get?key=my_test_key"
HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Date: Mon, 09 Apr 2018 21:45:14 GMT
Content-Length: 25

My any test string value.

curl -is "http://localhost:55555/del?key=my_test_key"
HTTP/1.1 200 OK
Date: Mon, 09 Apr 2018 21:45:43 GMT
Content-Length: 0
Content-Type: text/html; charset=ISO-8859-1

curl -is "http://localhost:55555/clean"
HTTP/1.1 200 OK
Date: Mon, 09 Apr 2018 21:46:14 GMT
Content-Length: 0
Content-Type: text/html; charset=ISO-8859-1

curl -is "http://localhost:55555/get?key=my_test_key"
HTTP/1.1 405 Method not allowed for this uri
Connection: close
Date: Mon, 09 Apr 2018 21:46:46 GMT
Content-Length: 0
Content-Type: text/html; charset=ISO-8859-1
```

Please take a look at the code. This is not the shortest of applications. But the created web service is very simple and located in the one source file.

I would like to say a few words about the application code.
- At the beginning of the file, you can see some of the included MIF headers.
- The main application class inherited from a predefined HTTP server application class.
- In the derived class, you must override the Init function. At this point, all http handlers are added, and you can add any other initialization of your own components.
- All handlers take two parameters: a const reference to the request package and a non constant reference to the response package. In the future, you will not do this. Everything will be easier.

Thus, you have a full-featured multi-threaded web server with an REST-like API, for simple data caching.

**All the code you will find in the example** 
[trivial_web_service](https://github.com/tdv/mif/tree/master/examples/_doc/chapters/chapter1/trivial_web_service)

# Simple HTTP echo-server

A fully-featured framework is a good basis for easy application building. However, sometimes you only need to use the http server component, for example, to embed into another application. The following code is a simple HTTP server with two handlers for creating an echo from an input request and calculating the number of echo-request requests.

The application can be created similarly to the previous one (clone, copy, mkdir, edit the source code and build script)
```cpp
// STD
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <string>

// MIF
#include <mif/common/unused.h>
#include <mif/net/http/server.h>

int main(int argc, char const **argv)
{
    try
    {
        if (argc != 4)
        {
            std::cout << "Usage: ./http_echo_server <host> <port> <workers>" << std::endl;
            std::cout << "For example: ./http_echo_server 0.0.0.0 55555 8" << std::endl;
            return EXIT_FAILURE;
        }

        std::atomic_size_t counter{0};

        Mif::Net::Http::Server server{argv[1], argv[2], static_cast<std::uint16_t>(std::stoi(argv[3])),
                {Mif::Net::Http::Method::Type::Post},
                {
                    {
                        "/echo", [&] (Mif::Net::Http::IInputPack const &req, Mif::Net::Http::IOutputPack &resp)
                        {
                            auto data = req.GetData();
                            resp.SetData(std::move(data));
                            ++counter;
                        }
                    },
                    {
                        "/stat", [&] (Mif::Net::Http::IInputPack const &req, Mif::Net::Http::IOutputPack &resp)
                        {
                            Mif::Common::Unused(req);
                            auto const value = "Counter: " + std::to_string(counter) + "\n";
                            resp.SetData({std::begin(value), std::end(value)});
                        }
                    }
                }
            };

        Mif::Common::Unused(server);

        std::cout << "Press any 'Enter' for quit." << std::endl;
        std::cin.get();
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
```
Run server:
```bash
./http_echo_server 0.0.0.0 55555 8
```
Tests:
```bash
curl -is -X POST "http://localhost:55555/echo" -d 'Test data.'
HTTP/1.1 200 OK
Date: Tue, 10 Apr 2018 09:16:34 GMT
Content-Length: 10
Content-Type: text/html; charset=ISO-8859-1

Test data.

curl -is -X POST "http://localhost:55555/stat"
HTTP/1.1 200 OK
Date: Tue, 10 Apr 2018 09:24:42 GMT
Content-Length: 11
Content-Type: text/html; charset=ISO-8859-1

Counter: 1
```

This is a very simple application in which you completely control the code. But this is not for creating systems based on microservices. This is for simple tasks, for example, for embedding in existing applications.

**All the code you will find in the example** 
[http_echo_server](https://github.com/tdv/mif/tree/master/examples/_doc/chapters/chapter1/http_echo_server)
And you can see one more in the [http_echo_server](https://github.com/tdv/mif/tree/master/examples/http_echo_server) example.