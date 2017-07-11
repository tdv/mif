//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/application.h>
#include <mif/common/log.h>
#include <mif/net/tcp_clients.h>

// COMMON
#include "common/client.h"
#include "common/ps/imy_company.h"

class Application
    : public Mif::Application::Application
{
public:
    Application(int argc, char const **argv)
        : Mif::Application::Application{argc, argv}
    {
        boost::program_options::options_description options{"Client options"};
        options.add_options()
                ("host", boost::program_options::value<std::string>()->default_value("0.0.0.0"), "Server host")
                ("port", boost::program_options::value<std::string>()->default_value("55555"), "Server port");

        AddCustomOptions(options);
    }

private:
    void ShowEmployees(Service::Data::Employees const &employees) const
    {
        for (auto const &e : employees)
        {
            MIF_LOG(Info) << "Employee. Id: " << e.first << " "
                << "Name: " << e.second.name << " "
                << "LastName: " << e.second.lastName << " "
                << "Age: " << e.second.age << " "
                << "Position: " << e.second.position;
        }
    }

    // Mif.Application.Application
    virtual void OnStart() override final
    {
        auto const &options = GetOptions();

        auto const host = options["host"].as<std::string>();
        auto const port = options["port"].as<std::string>();

        MIF_LOG(Info) << "Starting client on " << host << ":" << port;

        std::chrono::microseconds timeout{10 * 1000 * 1000};

        auto clientFactory = Service::Ipc::MakeClientFactory(4, timeout);

        Mif::Net::TCPClients clients(clientFactory);

        auto proxy = std::static_pointer_cast<Service::Ipc::ClientsChain>(clients.RunClient(host, port));

        MIF_LOG(Info) << "Client is successfully started.";

        auto client = proxy->GetClientItem<Service::Ipc::PSClient>();

        auto service = client->CreateService<Service::IMyCompany>("MyCompany");

        {
            Service::Data::Employee e;
            e.name = "Ivan";
            e.lastName = "Ivanov";
            e.age = 25;
            e.position = "manager";
            auto const eId = service->AddEmployee(e);
            MIF_LOG(Info) << "Employee Id: " << eId;
        }

        {
            Service::Data::Employee e;
            e.name = "Petr";
            e.lastName = "Petrov";
            e.age = 30;
            e.position = "developer";
            auto const eId = service->AddEmployee(e);
            MIF_LOG(Info) << "Employee Id: " << eId;
        }

        auto const &employees = service->GetEmployees();
        ShowEmployees(employees);

        if (!employees.empty())
        {
            auto id = std::begin(employees)->first;
            service->RemoveAccount(id);
            MIF_LOG(Info) << "Removed account " << id;
            auto const &employees = service->GetEmployees();
            ShowEmployees(employees);

            try
            {
                MIF_LOG(Info) << "Removed again account " << id;
                service->RemoveAccount(id);
            }
            catch (std::exception const &e)
            {
                MIF_LOG(Warning) << "Error: " << e.what();
            }
        }
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
