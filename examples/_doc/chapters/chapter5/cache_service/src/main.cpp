// MIF
#include <mif/application/http_server.h>
#include <mif/net/http/make_web_service.h>
#include <mif/service/root_locator.h>

// THIS
#include "id/service.h"

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
