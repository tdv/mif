//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
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
    Application(int argc, char const **argv)
        : HttpServer{argc, argv}
    {
    }

private:
    // Mif.Application.HttpService
    virtual void Init(Mif::Net::Http::ServerHandlers &handlers) override final
    {
        std::string const employeeLocation = "/employee";
        auto employeeService = Mif::Service::Make<Service::EmployeeService, Mif::Net::Http::IWebService>(employeeLocation);
        handlers.emplace(employeeLocation, Mif::Net::Http::MakeWebService(employeeService));
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
