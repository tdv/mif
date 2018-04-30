# Web service. Additional

Now you have all the knowledge necessary to handle HTTP requests. And you are ready to learn more about creating your own real web services. In this chapter, we'll look at a great example that looks like a real web service with many functions. The example from the chapter is a little contrived, but it is very close to many of my web services that I do in my real work. Well, in the beginning it is necessary to prepare a technical assignment for the web service. It will be a web service for caching data with access control for users. The web service must:
- provide a REST-like API
- cache data
- authorize user
- provide access control for users  

The following code is not simple, but in the future it will give more opportunities to create your own real web services.  
Project structure:  
src  
|--data  
|----meta  
|--id  
|--interface  
|--service  
|----handler  
|--version  

The src folder contains common files, such as main.cpp and some others and subfolders with some included and source files. Subfolders:  
- data - included files with user-defined data structures
- data/meta - additional meta-information for reflecting user data
- id - the services identifiers
- interface - user-defined interfaces
- services - implementations of user-defined interfaces
- ssrvice/handler - web service handlers
- version - version information  
User-defined common structures:
```cpp
// data/common.h
namespace CacheService
{
    namespace Data
    {

        using ID = std::string;
        using IDs = std::set<ID>;
        using StringMap = std::map<std::string, std::string>;
        using Timestamp = boost::posix_time::ptime;

        enum class Role
        {
            Admin,
            User
        };

        using Roles = std::set<Role>;

        struct Profile
        {
            std::string login;
            std::string password;
            std::string name;
            Roles roles;
            IDs buckets;
        };

        struct Session
        {
            ID id;
            Timestamp expires;
        };

    }   // namespace Data
}   // namespace CacheService

```
```cpp
// data/api.h
namespace CacheService
{
    namespace Data
    {
        namespace Api
        {
            namespace Response
            {

                struct Status
                {
                    std::int32_t code = 0;
                    std::string message;
                };

                struct Info
                {
                    std::string service;
                    std::string node;
                    std::string timestamp;
                    Status status;
                };

                struct Header
                {
                    Info meta;
                };

                struct Id
                {
                    Info meta;
                    ID data;
                };

                struct Data
                {
                    Info meta;
                    std::string data;
                };

                struct BucketKeys
                {
                    Info meta;
                    std::map<ID, IDs> data;
                };

                struct Profile
                {
                    Info meta;
                    CacheService::Data::Profile data;
                };

                struct ProfilesList
                {
                    Info meta;
                    StringMap data;
                };

            }   // namespace Response
        }   // namespace Api
    }   // namespace Data
}   // namespace CacheService
```
To use reflection, we need to add metainformation to user-defined structures  
```cpp
// data/meta/common.h
namespace CacheService
{
    namespace Data
    {
        namespace Meta
        {

            MIF_REFLECT_BEGIN(Role)
                MIF_REFLECT_FIELD(Admin)
                MIF_REFLECT_FIELD(User)
            MIF_REFLECT_END()

            MIF_REFLECT_BEGIN(Profile)
                MIF_REFLECT_FIELD(login)
                MIF_REFLECT_FIELD(password)
                MIF_REFLECT_FIELD(name)
                MIF_REFLECT_FIELD(roles)
                MIF_REFLECT_FIELD(buckets)
            MIF_REFLECT_END()

        }   // namespace Meta
    }   // namespace Data
}   // namespace CacheService

MIF_REGISTER_REFLECTED_TYPE(CacheService::Data::Meta::Role)
MIF_REGISTER_REFLECTED_TYPE(CacheService::Data::Meta::Profile)
```
For api.h, everything is done as for common.h  

In the previous chapters, I said that the web service and the MIF service are not equivalent. A service is an implementation of a user-defined interface, such as an abstract data structure in the C++ style, and all implementations must have a unique identifier. The id/service.h file contains these identifiers. To create unique identifiers that are independent of the position in the enums, we can use CRC32 from a some string.  
```cpp
namespace CacheService
{
    namespace Id
    {

        enum
        {
            DataFacade = Mif::Common::Crc32("CacheService.Id.DataFacade"),
            Auth = Mif::Common::Crc32("CacheService.Id.Auth"),
            Admin = Mif::Common::Crc32("CacheService.Id.Admin"),
            Cache = Mif::Common::Crc32("CacheService.Id.Cache")
        };

    }   // namespace Id
}   // namespace CacheService
```
Please take a look at the following code. This is the interface definition
```cpp
namespace CacheService
{

    struct IAdmin
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
        virtual Data::StringMap GetProfilesList() const = 0;
        virtual Data::Profile GetProfile(std::string const &login) const = 0;
        virtual void SetProfile(Data::Profile const &profile) = 0;
        virtual void RemoveProfile(std::string const &login) = 0;
    };

    using IAdminPtr = Mif::Service::TServicePtr<IAdmin>;

}   // namespace CacheService
```
Other interfaces of the service are similar to this one. Each implementation can implement one or more interfaces. The most interesting the DataFacade class, which implements three interfaces. Below you will see this class. All implementations of methods will be omitted, the class contains only method definition
```cpp
namespace CacheService
{
    namespace Detail
    {
        namespace
        {

            class DataFacade
                : public Mif::Service::Inherit
                    <
                        IAuth, IAdmin, ICache
                    >
            {
            public:
                DataFacade(Mif::Application::IConfigPtr config)
                {
                    if (!config)
                        throw std::invalid_argument{"DataFacade. No config."};
                    // ...
                }

            private:
                //IAuth
                virtual Data::Session Login(std::string const &login, std::string const &password) override final
                {
                    // ...
                }

                virtual void Logout(Data::ID const &session) override final
                {
                    // ...
                }

                virtual Data::Profile GetSessionProfile(Data::ID const &session) const override final
                {
                    // ...
                }

                //IAdmin
                virtual Data::StringMap GetProfilesList() const override final
                {
                    // ...
                }

                virtual Data::Profile GetProfile(std::string const &login) const override final
                {
                    // ...
                }

                virtual void SetProfile(Data::Profile const &profile) override final
                {
                    // ...
                }

                virtual void RemoveProfile(std::string const &login) override final
                {
                    // ...
                }

                // ICache
                virtual std::map<Data::ID, Data::IDs> ListBucketKeys(Data::IDs const &buckets) const override final
                {
                    // ...
                }

                virtual std::string GetData(Data::ID const &bucket, Data::ID const &key) const override final
                {
                    // ...
                }

                virtual void SetData(Data::ID const &bucket, Data::ID const &key,
                        std::string const &data) override final
                {
                    // ...
                }

                virtual void RemoveData(Data::ID const &bucket, Data::ID const &key) override final
                {
                    // ...
                }
            };

        }   // namespace
    }   // namespace Detail
}   // namespace CacheService

MIF_SERVICE_CREATOR
(
    ::CacheService::Id::DataFacade,
    ::CacheService::Detail::DataFacade,
    Mif::Application::IConfigPtr
)
```
At the end of the code, you saw a strange macro. This is the definition of the service creator. The entry point to the service. All included files with interfaces contain only service definitions. But many implementations can only be contained in cpp files. This allows you to make a more independent implementation code with deeper encapsulation and reduce compilation time.  
The MIF_SERVICE_CREATOR macro takes two required parameters and can take several optional parameters passed to the constructor class. The required parameters are the implementation identifier and the implementation itself. The DataFacade class as an optional parameter takes an smart pointer to configuration.  

All the web service handlers are inherited from the Base class, which contain common methods for all. The Base class is inherited from the WebService class, which is considered more than one time in the previous chapters. Below you will see the code of the most interesting handler
```cpp
namespace CacheService
{
    namespace Handler
    {
        namespace Detail
        {
            namespace
            {

                class Cache
                    : public Base
                {
                public:
                    Cache(std::string const &prefix)
                        : Base{prefix}
                    {
                        AddHandler("/list", this, &Cache::List);
                        AddHandler("/get", this, &Cache::Get);
                        AddHandler("/set", this, &Cache::Set);
                        AddHandler("/remove", this, &Cache::Remove);
                    }

                private:
                    // WebService.Hadlers
                    Response List(Headers const &headers)
                    {
                        auto const profile = CheckPermissions(headers, {Data::Role::User});

                        auto cache = Mif::Service::RootLocator::Get()->Get<ICache>(Id::DataFacade);

                        Data::Api::Response::BucketKeys response;

                        response.meta = GetMeta();
                        response.data = cache->ListBucketKeys(profile.buckets);

                        return response;
                    }

                    Response Get(Headers const &headers,
                                 Prm<Data::ID, Name("bucket")> const &bucket,
                                 Prm<Data::ID, Name("key")> const &key)
                    {
                        auto const profile = CheckPermissions(headers, {Data::Role::User});
                        CheckId(bucket, key);
                        CheckBucketId(bucket.Get(), profile.buckets);

                        auto cache = Mif::Service::RootLocator::Get()->Get<ICache>(Id::DataFacade);

                        Data::Api::Response::Data response;

                        response.meta = GetMeta();
                        response.data = cache->GetData(bucket.Get(), key.Get());

                        return response;
                    }

                    Response Set(Headers const &headers,
                                 Prm<Data::ID, Name("bucket")> const &bucket,
                                 Prm<Data::ID, Name("key")> const &key,
                                 Content<std::string> const &data)
                    {
                        auto const profile = CheckPermissions(headers, {Data::Role::User});
                        CheckId(bucket, key);
                        CheckBucketId(bucket.Get(), profile.buckets);

                        auto cache = Mif::Service::RootLocator::Get()->Get<ICache>(Id::DataFacade);
                        cache->SetData(bucket.Get(), key.Get(), data.Get());

                        Data::Api::Response::Header response;

                        response.meta = GetMeta();

                        return response;
                    }

                    Response Remove(Headers const &headers,
                                    Prm<Data::ID, Name("bucket")> const &bucket,
                                    Prm<Data::ID, Name("key")> const &key)
                    {
                        auto const profile = CheckPermissions(headers, {Data::Role::User});
                        CheckId(bucket, key);
                        CheckBucketId(bucket.Get(), profile.buckets);

                        auto cache = Mif::Service::RootLocator::Get()->Get<ICache>(Id::DataFacade);
                        cache->RemoveData(bucket.Get(), key.Get());

                        Data::Api::Response::Header response;

                        response.meta = GetMeta();

                        return response;
                    }

                    void CheckId(Prm<Data::ID, Name("bucket")> const &bucket,
                                 Prm<Data::ID, Name("key")> const &key) const
                    {
                        if (!bucket)
                            throw std::invalid_argument{"No \"bucket\" parameter."};
                        if (bucket.Get().empty())
                            throw std::invalid_argument{"\"bucket\" must not be empty."};

                        if (!key)
                            throw std::invalid_argument{"No \"key\" parameter."};
                        if (key.Get().empty())
                            throw std::invalid_argument{"\"key\" must not be empty."};
                    }

                    void CheckBucketId(Data::ID const &bucket, Data::IDs const &buckets) const
                    {
                        if (buckets.find(bucket) == std::end(buckets))
                            throw std::invalid_argument{"You can't working with bucket \"" + bucket + "\"."};
                    }
                };

            }   // namespace
        }   // namespace Detail
    }   // namespace Handler
}   // namespace CacheService

MIF_SERVICE_CREATOR
(
    ::CacheService::Id::Cache,
    ::CacheService::Handler::Detail::Cache,
    std::string
)
```
You can see an implementation that contains all the knowledge from the previous chapters by HTTP request handling. 

In this example, all web handlers are service implementations. And you need to create these implementations using their identifiers. The following code is the main entry point to the application. In it you will see the creation of handlers and their binding to locations, as well as the initialization of the data facade.
```cpp
namespace CacheService
{

    class Application
        : public Mif::Application::HttpServer
    {
    public:
        using HttpServer::HttpServer;

    private:
        virtual void Init(Mif::Net::Http::ServerHandlers &handlers) override final
        {
            auto config = GetConfig();
            if (!config)
                throw std::runtime_error{"No input config."};

            Mif::Service::RootLocator::Get()->Put<Id::DataFacade>(config->GetConfig("data_facade"));

            std::string const authLocation = "/auth";
            handlers.emplace(authLocation, Mif::Net::Http::MakeWebService<Id::Auth>(authLocation));

            std::string const adminLocation = "/admin";
            handlers.emplace(adminLocation, Mif::Net::Http::MakeWebService<Id::Admin>(adminLocation));

            std::string const cacheLocation = "/cache";
            handlers.emplace(cacheLocation, Mif::Net::Http::MakeWebService<Id::Cache>(cacheLocation));
        }
    };

}   // namespace CacheService

int main(int argc, char const **argv)
{
    return Mif::Application::Run<CacheService::Application>(argc, argv);
}
```
This code does not differ much from the previous applications. However, now to create web service handlers, you must use the MakeWebService function with the implementation ID, and not the implementation.  

**Service starting:**
```bash
./cache_service --config=../config/config.xml --daemon
```
In the above command line, you saw the configuration file and the "--daemon" option, Using the MIF application framework, you have the ability to run your own MIF-based application with a configuration from file. The optional parameter "--daemon" takes precedence over the configuration from the file and can be used to start the service in daemon mode.  
The configuration file can be in two formats:
- xml (default)
- json  

To use the json configuration file, you must use the "--configformat" option. More about the parameters of the program launch, you can see, running your MIF-based application with the parameter "--help". The configuration file must have two sections:
- common
- data  

A "common" section contains general information for starting an application based on MIF. The "data" section contains user data. However, in the "data" section, some MIF application templates contain their information to run. In the example above, the "data" section contains information about running the http server.  
Take a look at the following configuration file:
```xml
<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<document>
    <common>
        <daemon>0</daemon>
        <log>
            <level>4</level>
            <dir>.</dir>
            <pattern>log_%5N.log</pattern>
        </log>
    </common>
    <data>
        <server>
            <host>0.0.0.0</host>
            <port>55555</port>
            <workers>8</workers>
            <timeout>10000000</timeout>
        </server>
        <data_facade>
            <root>
                <login>root</login>
                <password>qwerty</password>
                <name>Root</name>
            </root>
        </data_facade>
    </data>
</document>
```
This configuration contains a common section and the data section contains information for running the http server and parameters for the user-defined "DataFacade" component,  
At the beginning of this chapter, it was said that the service should have access control for users. The data_facade section contains an account for the root user.  

**Tests.**  
When testing we'll login as root, add a couple of users with different permissions, and from each user we will try to cache the data.  

**Login:**
```bash
curl -is "http://localhost:55555/auth/login?login=root&password=qwerty"
HTTP/1.1 200 OK
Content-Type: application/json; charset=UTF-8
Set-Cookie: cache_service_session=205d7e04-75ed-42e8-86b9-abfb20e83ce1; Path=/; Expires=Thu, 26 Apr 2018 10:04:04 GMT;
Date: Tue, 24 Apr 2018 10:04:04 GMT
Content-Length: 225

{
	"data" : "205d7e04-75ed-42e8-86b9-abfb20e83ce1",
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : 0,
			"message" : ""
		},
		"timestamp" : "20180424T100404"
	}
}
```
Take a look at the above program output. In the header, you see the Set-Cookie. This is the "Cookie" that you will use in all of the following requests.  

**Add users:**
```bash
curl -is "http://localhost:55555/admin/profile/set" \  
-H "Cookie: cache_service_session=205d7e04-75ed-42e8-86b9-abfb20e83ce1;" \  
-d '{"login": "user1", "password": "123", "name": "User 1", "roles": ["User"], "buckets": ["bucket1", "bucket2"]}'  

HTTP/1.1 200 OK
Content-Type: application/json; charset=UTF-8
Date: Tue, 24 Apr 2018 10:14:01 GMT
Content-Length: 175

{
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : 0,
			"message" : ""
		},
		"timestamp" : "20180424T101401"
	}
}


curl -is "http://localhost:55555/admin/profile/set" \  
-H "Cookie: cache_service_session=205d7e04-75ed-42e8-86b9-abfb20e83ce1;" \  
-d '{"login": "user2", "password": "321", "name": "User 2", "roles": ["Admin", "User"], "buckets":  ["bucket2"]}'  

HTTP/1.1 200 OK
Content-Type: application/json; charset=UTF-8
Date: Tue, 24 Apr 2018 10:15:26 GMT
Content-Length: 175

{
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : 0,
			"message" : ""
		},
		"timestamp" : "20180424T101526"
	}
}
```
**List of users:**  
```bash
er5/cache_service/bin$ curl -is "http://localhost:55555/admin/profile/list" \  
-H "Cookie: cache_service_session=205d7e04-75ed-42e8-86b9-abfb20e83ce1;"  

HTTP/1.1 200 OK
Content-Type: application/json; charset=UTF-8
Date: Tue, 24 Apr 2018 10:22:38 GMT
Content-Length: 333

{
	"data" : 
	[
		{
			"id" : "root",
			"val" : "Root"
		},
		{
			"id" : "user1",
			"val" : "User 1"
		},
		{
			"id" : "user2",
			"val" : "User 2"
		}
	],
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : 0,
			"message" : ""
		},
		"timestamp" : "20180424T102238"
	}
}
```
**Get user account by login:**
```bash
curl -is "http://localhost:55555/admin/profile/get?login=user1" \  
-H "Cookie: cache_service_session=205d7e04-75ed-42e8-86b9-abfb20e83ce1;"  

HTTP/1.1 200 OK
Content-Type: application/json; charset=UTF-8
Date: Tue, 24 Apr 2018 10:25:08 GMT
Content-Length: 339

{
	"data" : 
	{
		"buckets" : 
		[
			"bucket1",
			"bucket2"
		],
		"login" : "user1",
		"name" : "User 1",
		"password" : "123",
		"roles" : 
		[
			"User"
		]
	},
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : 0,
			"message" : ""
		},
		"timestamp" : "20180424T102508"
	}
}
```
**Logout from the root user account:**  
```bashcurl -is "http://localhost:55555/auth/logout" \  
-H "Cookie: cache_service_session=205d7e04-75ed-42e8-86b9-abfb20e83ce1;"HTTP/1.1 200 OK  

Content-Type: application/json; charset=UTF-8
Date: Tue, 24 Apr 2018 10:27:51 GMT
Content-Length: 175

{
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : 0,
			"message" : ""
		},
		"timestamp" : "20180424T102751"
	}
}
```
**Working with data caching:**
```bash
curl -is "http://localhost:55555/auth/login?login=user1&password=123"  

HTTP/1.1 200 OK
Content-Type: application/json; charset=UTF-8
Set-Cookie: cache_service_session=983b61fb-1108-4c9d-8def-19d0635d9adb; Path=/; Expires=Thu, 26 Apr 2018 10:32:55 GMT;
Date: Tue, 24 Apr 2018 10:32:55 GMT
Content-Length: 225

{
	"data" : "983b61fb-1108-4c9d-8def-19d0635d9adb",
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : 0,
			"message" : ""
		},
		"timestamp" : "20180424T103255"
	}
}

curl -is "http://localhost:55555/cache/set?bucket=buchet1&key=test" \  
-H "Cookie: cache_service_session=983b61fb-1108-4c9d-8def-19d0635d9adb;" \  
-d 'Test data'  

HTTP/1.1 400 Invalid http request was made
Connection: close
Content-Type: application/json; charset=UTF-8
Date: Tue, 24 Apr 2018 10:34:20 GMT
Content-Length: 218

{
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : -1,
			"message" : "You can't working with bucket \"buchet1\"."
		},
		"timestamp" : "20180424T103420"
	}
}

curl -is "http://localhost:55555/cache/set?bucket=bucket1&key=test" \  
-H "Cookie: cache_service_session=983b61fb-1108-4c9d-8def-19d0635d9adb;" \  
-d 'Test data'  

HTTP/1.1 200 OK
Content-Type: application/json; charset=UTF-8
Date: Tue, 24 Apr 2018 10:34:36 GMT
Content-Length: 175

{
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : 0,
			"message" : ""
		},
		"timestamp" : "20180424T103436"
	}
}

curl -is "http://localhost:55555/cache/get?bucket=bucket1&key=test" \  
-H "Cookie: cache_service_session=983b61fb-1108-4c9d-8def-19d0635d9adb;"  

HTTP/1.1 200 OK
Content-Type: application/json; charset=UTF-8
Date: Tue, 24 Apr 2018 10:34:52 GMT
Content-Length: 198

{
	"data" : "Test data",
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : 0,
			"message" : ""
		},
		"timestamp" : "20180424T103452"
	}
}

curl -is "http://localhost:55555/cache/remove?bucket=bucket1&key=test" \  
-H "Cookie: cache_service_session=983b61fb-1108-4c9d-8def-19d0635d9adb;"  

HTTP/1.1 200 OK
Content-Type: application/json; charset=UTF-8
Date: Tue, 24 Apr 2018 10:35:05 GMT
Content-Length: 175

{
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : 0,
			"message" : ""
		},
		"timestamp" : "20180424T103505"
	}
}

curl -is "http://localhost:55555/cache/get?bucket=bucket1&key=test" \  
-H "Cookie: cache_service_session=983b61fb-1108-4c9d-8def-19d0635d9adb;"  

HTTP/1.1 404 Could not find content for uri
Connection: close
Content-Type: application/json; charset=UTF-8
Date: Tue, 24 Apr 2018 10:35:10 GMT
Content-Length: 231

{
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : -1,
			"message" : "Data for bucket \"bucket1\" and key \"test\" not found."
		},
		"timestamp" : "20180424T103510"
	}
}

curl -is "http://localhost:55555/cache/set?bucket=bucket1&key=test" \  
-H "Cookie: cache_service_session=983b61fb-1108-4c9d-8def-19d0635d9adb;" \  
-d 'Test data'  

HTTP/1.1 200 OK
Content-Type: application/json; charset=UTF-8
Date: Tue, 24 Apr 2018 10:35:27 GMT
Content-Length: 175

{
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : 0,
			"message" : ""
		},
		"timestamp" : "20180424T103527"
	}
}

curl -is "http://localhost:55555/cache/list" \  
-H "Cookie: cache_service_session=983b61fb-1108-4c9d-8def-19d0635d9adb;"  

HTTP/1.1 200 OK
Content-Type: application/json; charset=UTF-8
Date: Tue, 24 Apr 2018 10:35:41 GMT
Content-Length: 255

{
	"data" : 
	[
		{
			"id" : "bucket1",
			"val" : 
			[
				"test"
			]
		}
	],
	"meta" : 
	{
		"node" : "localhost",
		"service" : "cache_service 1.0.0",
		"status" : 
		{
			"code" : 0,
			"message" : ""
		},
		"timestamp" : "20180424T103541"
	}
}
```

In the above test service usage protocol, you can see all the operations of data caching and some results of access control. Try the same steps for user2 and compare the results.  

The Base class has some interesting code that was not considered in this chapter. For example, the OnException method, which allows you to perform custom error handling. You can also see many interesting things in the source code of the example and in other examples, such as [http_crud](https://github.com/tdv/mif/tree/master/examples/http_crud) and [microservices](https://github.com/tdv/mif/tree/master/examples/microservices). They are very close to the real web services that are used in real projects.  


**All the code you will find in the example** [cache_service](https://github.com/tdv/mif/tree/master/examples/_doc/chapters/chapter5/cache_service)