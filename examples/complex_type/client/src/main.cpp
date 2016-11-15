//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>

// BOOST
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

// MIF
#include <mif/net/client_factory.h>
#include <mif/net/tcp_clients.h>
#include <mif/remote/proxy_client.h>
#include <mif/remote/serialization/serialization.h>
#include <mif/remote/serialization/boost.h>

// COMMON
#include "common/ps/imy_company.h"
#include "common/protocol_chain.h"

void ShowEmployees(Data::Employees const &employees)
{
    for (auto const &e : employees)
    {
        std::cout << "Employee. Id: " << e.first << " "
            << "Name: " << e.second.name << " "
            << "LastName: " << e.second.lastName << " "
            << "Age: " << e.second.age << " "
            << "Position: " << e.second.position << std::endl;
    }
}

int main(int argc, char const **argv)
{
    if (argc != 3)
    {
        std::cerr << "Bad params. Usage: complextype_client <host> <port>" << std::endl;
        return -1;
    }
    try
    {
        using BoostSerializer = Mif::Remote::Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
        using BoostDeserializer = Mif::Remote::Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
        using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<BoostSerializer, BoostDeserializer>;

        using ProxyClient = Mif::Remote::ProxyClient<SerializerTraits, IMyCompany_PS>;

        using ClientsChain = ProtocolChain<ProxyClient>;

        using ProxyFactory = Mif::Net::ClientFactory<ClientsChain>;

        std::chrono::microseconds timeout{10 * 1000 * 1000};

        std::cout << "Starting client on \"" << argv[1] << ":" << argv[2] << "\"" << std::endl;

        auto clientFactgory = std::make_shared<ProxyFactory>
            (
                Mif::Common::MakeCreator<ProxyClient>(timeout)
            );

        Mif::Net::TCPClients clients(4, clientFactgory);

        auto proxy = std::static_pointer_cast<ClientsChain>(clients.RunClient(argv[1], argv[2]));

        auto client = proxy->GetClientItem<ProxyClient>();

        auto service = client->CreateService<IMyCompany>("MyCompanyService");

        std::cout << "Client started." << std::endl;

        {
            Data::Employee e;
            e.name = "Ivan";
            e.lastName = "Ivanov";
            e.age = 25;
            e.position = "manager";
            auto const eId = service->AddEmployee(e);
            std::cout << "Employee Id: " << eId << std::endl;
        }

        {
            Data::Employee e;
            e.name = "Petr";
            e.lastName = "Petrov";
            e.age = 30;
            e.position = "developer";
            auto const eId = service->AddEmployee(e);
            std::cout << "Employee Id: " << eId << std::endl;
        }

        auto const &employees = service->GetEmployees();
        ShowEmployees(employees);

        if (!employees.empty())
        {
            auto id = std::begin(employees)->first;
            service->RemoveAccount(id);
            std::cout << "Removed account " << id << std::endl;
            auto const &employees = service->GetEmployees();
            ShowEmployees(employees);

            try
            {
                std::cout << "Removed again account " << id << std::endl;
                service->RemoveAccount(id);
            }
            catch (std::exception const &e)
            {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }

        std::cout << "Press Enter for quit." << std::endl;

        std::cin.get();

        std::cout << "Client stopped." << std::endl;
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
