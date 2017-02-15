//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>

// MIF
#include <mif/net/tcp_clients.h>

// COMMON
#include "common/client.h"
#include "common/ps/iface.h"

int main(int argc, char const **argv)
{
    if (argc != 3)
    {
        std::cerr << "Bad params. Usage: protostack_client <host> <port>" << std::endl;
        return -1;
    }
    try
    {
        std::chrono::microseconds const timeout{10 * 1000 * 1000};

        std::cout << "Starting client on \"" << argv[1] << ":" << argv[2] << "\"" << std::endl;

        auto clientFactory = Service::Ipc::MakeClientFactory(4, timeout);

        Mif::Net::TCPClients clients(clientFactory);

        auto proxy = std::static_pointer_cast<Service::Ipc::ClientsChain>(clients.RunClient(argv[1], argv[2]));

        auto client = proxy->GetClientItem<Service::Ipc::PSClient>();

        auto service = client->CreateService<Service::IEmployee>("Service");

        std::cout << "Client started." << std::endl;

        auto human = service->Cast<Service::IHuman>();
        std::cout << "Name: \"" << human->GetName() << "\"" << std::endl;
        std::cout << "Age: \"" << human->GetAge() << "\"" << std::endl;

        service->Planning();
        service->Development();
        service->BugFixes();
        service->Monitoring();

        std::cout << "Rate: \"" << service->GetRate() << "\"" << std::endl;

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
