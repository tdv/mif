# Simple web service

In Chapter 1, we considered two simple web services. This chapter shows other capabilities of the framework. Please take a look at the following code:
```cpp
// STD
#include <atomic>
#include <string>

// MIF
#include <mif/application/http_server.h>
#include <mif/net/http/web_service.h>
#include <mif/net/http/make_web_service.h>

class EchoService
    : public Mif::Net::Http::WebService
{
public:
    EchoService(std::string const &locationPrefix)
    {
        AddHandler(locationPrefix + "", this, &EchoService::Echo);
        AddHandler(locationPrefix + "/stat", this, &EchoService::Stat);
    }

private:
    std::atomic_size_t m_counter{0};

    std::string Echo(Content<std::string> const &content)
    {
        ++m_counter;
        return "Echo from server: \"" + content.Get() + "\"\n";
    }

    std::string Stat()
    {
        return "Count of echo-request: " + std::to_string(m_counter) + "\n";
    }
};

class Application
    : public Mif::Application::HttpServer
{
public:
    using HttpServer::HttpServer;

private:
    virtual void Init(Mif::Net::Http::ServerHandlers &handlers) override final
    {
        auto const echoServiceLocation = "/echo";
        auto echoService = Mif::Net::Http::MakeWebService<EchoService>(echoServiceLocation);
        handlers.emplace(echoServiceLocation, std::move(echoService));
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
```
This is a very simple web service created similar to the services in Chapter 1. But in this service you do not need to work with request and response data packs. All of the works by requests routing, extracting content, etc. In the depths of MIF. You effectively write only code of the requests handlers in C ++ and do not think about http, data extracting from a request body, data serialization and url query params parsing. In the example, not many methods for processing the parameters and the body of the query are considered, and also are not considered the mechanisms of the response data serialization. All of this will be considered in the next chapters.  

A few words about the code:
- All web service classes must be inherited from the WebService class
- The WebService is a service.
- Web service and service in MIF are not equivalently.
- The WebService is a abstract class inherited from the IService interface. This class contains methods for registering request handlers and performs a lot of work on processing requests and responses.
- WebService is abstract, you can not create it with the NEW operator. To create an instance, you must use the MakeWebService method from MIF
- By default, all handlers can return types that can be written to std::ostream. But you can use serialization in json, xml, and so on. This will be considered next time. However, if you do not need to return data, you can use void as the returning type.
- Arguments of handlers can not be of arbitrary type. In the following chapters, all types of arguments of handlers will be considered. These types are intended for parsing URLs, processing of http headers and deserialize the request body.
- All handlers must be created in the Init method. And another initialization of your own components should be performed in this method.  
- In the Init method, you can add a lot of resources. And each of the resources can be route by a subpaths for handling. Use AddHandler for this in the constructor of the service class.

Run service:
```bash
./simple_web_service
```

Tests:
```bash
curl -is "http://localhost:55555/echo" -d "Test data."
HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Date: Tue, 10 Apr 2018 22:19:52 GMT
Content-Length: 31

Echo from server: "Test data."

curl -is "http://localhost:55555/echo/stat"
HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Date: Tue, 10 Apr 2018 22:20:42 GMT
Content-Length: 25

Count of echo-request: 1
```

**All the code you will find in the example** 
[simple_web_service](https://github.com/tdv/mif/tree/master/examples/_doc/chapters/chapter2/simple_web_service)
