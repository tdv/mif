//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/tcp_service_client.h>
#include <mif/common/log.h>

// COMMON
#include "common/id/service.h"
#include "common/ps/imy_company.h"

class Application
    : public Mif::Application::TcpServiceClient
{
public:
    using TcpServiceClient::TcpServiceClient;

private:
    void ShowEmployees(Service::Data::Employees const &employees) const
    {
        for (auto const &e : employees)
        {
            MIF_LOG(Info) << "Employee. Id: " << e.first << " "
                << "Name: " << e.second.name << " "
                << "LastName: " << e.second.lastName << " "
                << "Age: " << e.second.age << " "
                << "Position: " << Mif::Reflection::ToString(e.second.position);
        }
    }

    // Mif.Application.TcpServiceClient
    virtual void Init(Mif::Service::IFactoryPtr factory) override final
    {
        auto service = factory->Create<Service::IMyCompany>(Service::Id::MyCompany);

        {
            Service::Data::Employee e;
            e.name = "Ivan";
            e.lastName = "Ivanov";
            e.age = 25;
            e.position = Service::Data::Position::Manager;
            auto const eId = service->AddEmployee(e);
            MIF_LOG(Info) << "Employee Id: " << eId;
        }

        {
            Service::Data::Employee e;
            e.name = "Petr";
            e.lastName = "Petrov";
            e.age = 30;
            e.position = Service::Data::Position::Developer;
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
