//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/http_server.h>
#include <mif/net/http/make_web_service.h>

// THIS
#include "id/service.h"

class Application
    : public Mif::Application::HttpServer
{
public:
    using HttpServer::HttpServer;

private:
    // Mif.Application.HttpService
    virtual void Init(Mif::Net::Http::ServerHandlers &handlers) override final
    {
        auto config = GetConfig();
        if (!config)
            throw std::runtime_error{"[Service::Application::Init] No config."};

        std::string const employeeLocation = "/employee";
        auto employeeHandler = Mif::Net::Http::MakeWebService<Service::Id::Employee>(
                employeeLocation, config->GetConfig("storage")
            );

        handlers.emplace(employeeLocation, employeeHandler);
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
