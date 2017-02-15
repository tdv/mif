//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>

// MIF
#include <mif/net/tcp_server.h>

// COMMON
#include "common/client.h"
#include "common/id/service.h"
#include "common/ps/imy_company.h"

int main(int argc, char const **argv)
{
    if (argc != 3)
    {
        std::cerr << "Bad params. Usage: complextype_server <host> <port>" << std::endl;
        return -1;
    }
    try
    {
        auto factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::Factory>();
        factory->AddClass<::Service::Id::MyCompany>();

        std::cout << "Starting server on \"" << argv[1] << ":" << argv[2] << "\" ..." << std::endl;

        std::chrono::microseconds timeout{10 * 1000 * 1000};

        auto clientFactory = Service::Ipc::MakeClientFactory(4, timeout, factory);

        auto server = std::make_shared<Mif::Net::TCPServer>(
            argv[1], argv[2], clientFactory);
        (void)server;
        
        std::cout << "Started server. Press Enter for quit." << std::endl;
        
        std::cin.get();
        
        std::cout << "Server stopped." << std::endl;

    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
