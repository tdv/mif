//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>

// MIF
#include <mif/net/http_clients.h>

// COMMON
#include "common/client.h"
#include "common/ps/ihello_world.h"

#include <thread>
#include <list>

int main(int argc, char const **argv)
{
    if (argc != 3)
    {
        std::cerr << "Bad params. Usage: helloworld_client <host> <port>" << std::endl;
        return -1;
    }
    try
    {
        std::chrono::microseconds const timeout{10 * 1000 * 1000};

        std::cout << "Starting client on \"" << argv[1] << ":" << argv[2] << "\"" << std::endl;

        auto clientFactory = Service::Ipc::MakeClientFactory(4, timeout);

        Mif::Net::HTTPClients clients(clientFactory);

        auto proxy = std::static_pointer_cast<Service::Ipc::ClientsChain>(clients.RunClient(argv[1], argv[2]));

        auto client = proxy->GetClientItem<Service::Ipc::PSClient>();

        auto service = client->CreateService<Service::IHelloWorld>("HelloWorld");

        std::cout << "Client started." << std::endl;

        std::cout << "Add words." << std::endl;

        service->AddWord("Hello");
        service->AddWord("World");
        service->AddWord("!!!");

        std::cout << "Result from server: \"" << service->GetText() << "\"" << std::endl;
        
        std::cout << "Clean." << std::endl;
        service->Clean();

        std::list<std::thread> threads;
        for (int j = 0 ; j < 50 ; ++j)
        {
            std::thread t([&]
                {
                    for (int i = 0 ; i < 50000 ; ++i)
                    try
                    {
                        /*std::cout << "Result from server: \"" << */service->GetText() /*<< "\"" << std::endl*/;
                    }
                    catch (std::exception const &e)
                    {
                        std::cerr << "Error: " << e.what() << std::endl;
                        break;
                    }
                }
            );
            threads.push_back(std::move(t));
        }

        std::cout << "Press Enter for quit." << std::endl;

        std::cin.get();

        for (auto &t : threads)
            t.join();

        std::cout << "Client stopped." << std::endl;
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
