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
#include "employee_service.h"

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
            throw std::runtime_error{"[Application::Init] No config."};
        auto dbConfig = config->GetConfig("database");
        if (!dbConfig)
            throw std::runtime_error{"[Application::Init] No 'database' node in the config."};
        std::string const employeeLocation = "/employee";
        auto employeeService = Mif::Service::Make<Service::EmployeeService, Mif::Net::Http::IWebService>(
                employeeLocation, dbConfig);
        handlers.emplace(employeeLocation, Mif::Net::Http::MakeWebService(employeeService));
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
