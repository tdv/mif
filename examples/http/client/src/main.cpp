//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>

// MIF
#include <mif/net/http_clients.h>

// COMMON
#include "common/client.h"
#include "common/ps/iadmin.h"

int main(int argc, char const **argv)
{
    if (argc != 3)
    {
        std::cerr << "Bad params. Usage: http_client <host> <port>" << std::endl;
        return -1;
    }
    try
    {
        std::chrono::microseconds const timeout{10 * 1000 * 1000};

        std::cout << "Starting client on \"" << argv[1] << ":" << argv[2] << "\"" << std::endl;

        auto clientFactory = Service::Ipc::MakeWebClientFactory(timeout);

        Mif::Net::HTTPClients clients(clientFactory);

        auto proxy = std::static_pointer_cast<Service::Ipc::ClientsChain>(clients.RunClient(argv[1], argv[2], "/admin"));

        auto client = proxy->GetClientItem<Service::Ipc::WebPSClient>();

        auto service = client->CreateService<Service::IAdmin>("WebService.Admin");

        std::cout << "Client started." << std::endl;

        std::cout << "Add words." << std::endl;

        service->SetTitle("Client title");
        service->SetBody("Client body");

        std::cout << "Page preview: \n" << service->GetPage() << std::endl;
        
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
