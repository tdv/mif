//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>

// BOOST
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

// MIF
#include <mif/net/client_factory.h>
#include <mif/net/tcp_server.h>
#include <mif/remote/serialization/serialization.h>
#include <mif/remote/serialization/boost.h>
#include <mif/service/make.h>
#include <mif/service/factory.h>

// COMMON
#include "common/ps/ihello_world.h"

// MIF
#include <mif/remote/stub_client.h>

// THIS
#include "common/id/service.h"

int main(int argc, char const **argv)
{
    if (argc != 3)
    {
        std::cerr << "Bad params. Usage: helloworld_server <host> <port>" << std::endl;
        return -1;
    }
    try
    {
        using BoostSerializer = Mif::Remote::Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
        using BoostDeserializer = Mif::Remote::Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
        using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<BoostSerializer, BoostDeserializer>;

        auto factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::Factory>();
        factory->AddClass<Service::Id::HelloWorld>();

        using StubClient = Mif::Remote::StubClient<SerializerTraits>;
        using StubFactory = Mif::Net::ClientFactory<StubClient>;

        std::cout << "Starting server on \"" << argv[1] << ":" << argv[2] << "\" ..." << std::endl;

        auto netClientFactgory = std::make_shared<StubFactory>(factory);
        auto server = std::make_shared<Mif::Net::TCPServer>(
            argv[1], argv[2], 4, netClientFactgory);
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
