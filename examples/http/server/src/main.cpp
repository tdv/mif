//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/http_server.h>
#include <mif/net/http/servlet.h>
#include <mif/net/http/make_web_service.h>

// COMMON
#include "common/client.h"
#include "common/id/service.h"
#include "common/ps/iadmin.h"

class Application
    : public Mif::Application::HttpServer
{
public:
    using HttpServer::HttpServer;

private:
    // Mif.Application.HttpService
    virtual void Init(Mif::Net::Http::ServerHandlers &handlers) override final
    {
        std::string const adminLocation = "/admin";
        std::string const viewLocation = "/view";

        auto service = Mif::Service::Create<Service::Id::Service>(viewLocation);
        auto webService = Mif::Service::Cast<Mif::Net::Http::IWebService>(service);

        auto factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::Factory>();
        factory->AddInstance(Service::Id::Service, service);

        std::chrono::microseconds const timeout{10 * 1000 * 1000};

        auto clientFactory = Service::Ipc::MakeClientFactory(timeout, factory);

        handlers.emplace(adminLocation, Mif::Net::Http::MakeServlet(clientFactory));
        handlers.emplace(viewLocation, Mif::Net::Http::MakeWebService(webService));
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
