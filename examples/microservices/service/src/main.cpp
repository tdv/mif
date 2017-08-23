//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/http_server.h>
#include <mif/net/http/make_web_service.h>
#include <mif/remote/predefined/utility.h>

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
        auto stgConfig = config->GetConfig("storage");
        if (!stgConfig)
            throw std::runtime_error{"[Service::Application::Init] No 'storage' config node."};

        auto const host = stgConfig->GetValue("host");
        auto const port = stgConfig->GetValue("port");
        auto const workers = stgConfig->GetValue<std::uint16_t>("workers");
        std::chrono::microseconds const timeout{stgConfig->GetValue<std::uint32_t>("timeout")};

        auto remoteFactory = Mif::Remote::Predefined::CreateTcpClientServiceFactory(
                host, port, workers, timeout
            );

        std::string const employeeLocation = "/employee";
        auto employeeHandler = Mif::Net::Http::MakeWebService<Service::Id::Employee>(
                employeeLocation, remoteFactory
            );

        handlers.emplace(employeeLocation, employeeHandler);
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
