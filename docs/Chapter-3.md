# Web service. Parameters processing

This chapter is a continuation of the previous chapter. This chapter will be considered mechanisms for parsing URL parameters and http-headers processing. Take a look at the following code
```cpp
class EchoService
    : public Mif::Net::Http::WebService
{
public:
    EchoService(std::string const &locationPrefix)
    {
        AddHandler(locationPrefix + "/text", this, &EchoService::EchoText);
        AddHandler(locationPrefix + "/headers", this, &EchoService::EchoHeaders);
        AddHandler(locationPrefix + "/params", this, &EchoService::EchoParams);
        AddHandler(locationPrefix + "/prm", this, &EchoService::EchoPrm);
        AddHandler(locationPrefix + "/stat", this, &EchoService::Stat);
    }

private:
    std::string EchoText(Content<std::string> const &content)
    {
        return "Echo from server: \"" + content.Get() + "\"\n";
    }

    std::string EchoHeaders(Headers const &headers)
    {
        return "Echo from server:\n" + ToString(headers.Get()) + "\n";
    }

    std::string EchoParams(Params const &params)
    {
        return "Echo from server:\n" + ToString(params.Get()) + "\n";
    }

    std::string EchoPrm(Prm<std::string, Name("prm1")> const &prm1,
                        Prm<int, Name("prm2")> const &prm2,
                        Prm<std::set<std::string>, Name("prm3")> const &prm3)
    {
        if (!prm1)
            throw std::invalid_argument{"No prm1."};
        if (!prm2)
            throw std::invalid_argument{"No prm2."};
        if (!prm3)
            throw std::invalid_argument{"No prm3."};
        return "Echo from server:\n"
                "\tPrm1: " + prm1.Get() + "\n"
                "\tPrm2: " + std::to_string(prm2.Get()) + "\n"
                "\tPrm3: (" + boost::algorithm::join(prm3.Get(), ",") + ")\n";
    }

    std::string Stat()
    {
        std::set<std::string> items;
        auto const &stat = GetStatistics();
        std::transform(std::begin(stat.resources), std::end(stat.resources),
                std::inserter(items, std::begin(items)),
                [] (typename std::decay<decltype(stat.resources)>::type::value_type const &i)
                {
                    std::string item = "Resource: " + i.first + "\n";
                    item += "\tTotal: " + std::to_string(i.second.total) + "\n";
                    item += "\tBad: " + std::to_string(i.second.bad) + "\n";
                    return item;
                }
            );
        return boost::algorithm::join(items, "\n");
    }

    std::string ToString(std::map<std::string, std::string> const &map) const
    {
        std::set<std::string> items;
        std::transform(std::begin(map), std::end(map),
                std::inserter(items, std::begin(items)),
                [] (typename std::decay<decltype(map)>::type::value_type const &i)
                {
                    return i.first + ": " + i.second;
                }
            );
        return boost::algorithm::join(items, "\n");
    }
};
```
This is a handler designed to demonstrate the capabilities of URL parameters processing and http-headers. This is not all of the web service code. The web service is similar to the code from the previous chapter and is not interesting here. The handler can return of:
- plain text from the request body
- all headers in json format
- all URL query parameters in json format
- some parameters that were parsed  and cast to particular types
- statistics of the number of request made

Parameters of all handlers are not arbitrary. Each of the parameters can be one of the following types:
- Content
- Headers
- Params
- Prm
- ParamPack  

These are the classes defined in the WebService class. The next part of the chapter will be to consider each of classes

## Content
This class is intended to extract data from the body of the http request. The chapter considers a very simple example demonstrating the extraction of data from the body of the http request as raw string. But this class can be used in more complex solutions. For example, to extract data from the body and automatically convert to one of the user-defined reflectable structures. Working with reflectable user-defined data structures will be considered in the next chapter. To extract the reflectable user-defined data structures, you can use any deserializer from any data formats. MIF allows you to use Json, Xml data serialization. But you can make your serialization in the needed for you data format.

## Headers
The Headers class is a simple class similar to boost::optional, which provides access to HTTP request headers like as map of strings. It can be used to simply search for one of the request headers you need.

## Params
Params is a simple class, similar to the Headers class and intended to simple access the parameters of a URL request. All parameters provided by the Params class can be obtained as strings without casting to the desired type.

## Prm
This class allows you to do a lot of interesting things. And this class already allows to extract  by name parameters from URLs with automatic casting to the desired type. This class works like boost::optional and can be used to process optional parameters. When you declare a parameter of type Prm, you must specify a particular type for casting and name of parameter. You can use the following types:
- std::string
- all integral types
- a float pointing types
- containers such as std::list, std::vector, std::set (without complex value types for containers similar to std::map). As a delimiter for the values ​​of the container elements in the URL, you must use semicolon.
- date and time types from boost::posix_time.  

In addition you can create your own URL parameter parser.  

The following code demonstrates how to work with date and time types from the boost library.
```cpp
using Date = boost::posix_time::ptime::date_type;
using Time = boost::posix_time::ptime::time_duration_type;
using Timestamp = boost::posix_time::ptime;

// ....

void AnyHandler(Prm<Date, Name("d")> const &date,
        Prm<Time, Name("t")> const &time,
        Prm<Timestamp, Name("ts")> const &ts)
{
    // ...
}
```
Call handler:
```bash
curl "http://host:port/path/hadler?d=2018.04.22&t=14:55:33&ts=2018.04.22%2014:55:33"
```

## ParamPack
The ParamPack class, intended for retrieve typed parameters from a URLs query, for example, as a reflectable user-defined data structure. This class can be used as an alternative way to use the Prm class with complex types.
```cpp
struct Phones
{
    std::string name;
    boost::optional<std::list<std::string>> phones;
};

// Add information for reflection

void AnyHandler(ParamPack<Phones> const &data)
{
    if (!data)
        return;

    auto const &info = data.Get();
    MIF_LOG(Info) << "Name: " << info.name;

    if (!info.phones)
        return;

    for (auto const &i : info.phones.get())
        MIF_LOG(Info) << "Phone: " << I;
}
```
Call handler:
```bash
curl "http://host:port/path/hadler?name=John&phones=(212)xxx-xxx;(212)yyy-yyyy"
```

Well. Now we are ready to build, run and test the example as a whole. To build and run the example, you can do 
steps similar to the steps from previous chapters.  
Run service in daemon mode
```bash
./advanced_echo_service --daemon
```
**Note:** running the service with option --help you will get more information about the program startup parameters.  

Tests:
```bash
curl -is "http://localhost:55555/echo/text" -d 'Test data.'
HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Date: Tue, 17 Apr 2018 14:49:13 GMT
Content-Length: 31

Echo from server: "Test data."


curl -is "http://localhost:55555/echo/headers" -H "My-User-Defined-Header: the test value of user-defined header."
HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Date: Tue, 17 Apr 2018 14:50:40 GMT
Content-Length: 139

Echo from server:
Accept: */*
Host: localhost:55555
My-User-Defined-Header: the test value of user-defined header.
User-Agent: curl/7.47.0


curl -is "http://localhost:55555/echo/params?prm1=value1&prm2=value2"
HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Date: Tue, 17 Apr 2018 14:51:57 GMT
Content-Length: 44

Echo from server:
prm1: value1
prm2: value2


curl -is "http://localhost:55555/echo/prm?prm1=string%20value&prm2=100500&prm3=1;2;3;4;5;6;7;8;9;0"
HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Date: Tue, 17 Apr 2018 14:53:53 GMT
Content-Length: 81

Echo from server:
	Prm1: string value
	Prm2: 100500
	Prm3: (0,1,2,3,4,5,6,7,8,9)


curl -is "http://localhost:55555/echo/stat"
HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Date: Tue, 17 Apr 2018 14:54:37 GMT
Content-Length: 203

Resource: /echo/headers
	Total: 1
	Bad: 0

Resource: /echo/params
	Total: 2
	Bad: 0

Resource: /echo/prm
	Total: 1
	Bad: 0

Resource: /echo/stat
	Total: 1
	Bad: 0

Resource: /echo/text
	Total: 1
	Bad: 0
```
Service stopping
```bash
sudo killall -w advanced_echo_service
```

**All the code you will find in the example** 
[advanced_echo_service](https://github.com/tdv/mif/tree/master/examples/_doc/chapters/chapter3/advanced_echo_service)