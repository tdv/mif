//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>

// MIF
#include <mif/net/http/server.h>
#include <mif/net/http/servlet.h>
#include <mif/net/http/make_web_service.h>

// COMMON
#include "common/client.h"
#include "common/ps/iadmin.h"

// THIS
#include "common/id/service.h"

int main(int argc, char const **argv)
{
    if (argc != 3)
    {
        std::cerr << "Bad params. Usage: http_server <host> <port>" << std::endl;
        return -1;
    }
    try
    {
        std::string const adminLocation = "/admin";
        std::string const viewLocation = "/view";

        auto service = Mif::Service::Create<Service::Id::Service>(viewLocation);
        auto webService = Mif::Service::Cast<Mif::Net::Http::IWebService>(service);

        auto factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::Factory>();
        factory->AddInstance(Service::Id::Service, service);

        std::cout << "Starting server on \"" << argv[1] << ":" << argv[2] << "\" ..." << std::endl;

        std::chrono::microseconds const timeout{10 * 1000 * 1000};

        auto clientFactory = Service::Ipc::MakeWebClientFactory(timeout, factory);

        Mif::Net::Http::Server server{argv[1], argv[2], 8,
                {Mif::Net::Http::Method::Type::Get, Mif::Net::Http::Method::Type::Post},
                {
                    {adminLocation, Mif::Net::Http::MakeServlet(clientFactory)},
                    {viewLocation, Mif::Net::Http::MakeWebService(webService)}
                }
            };
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
