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
#include <mif/net/tcp_server.h>
#include <mif/remote/serialization/serialization.h>
#include <mif/remote/serialization/boost/serialization.h>
#include <mif/remote/stub_client.h>
#include <mif/service/service_factory.h>

// COMMON
#include "common/ps/imy_company.h"
#include "common/protocol_chain.h"

// THIS
#include "my_company_service.h"

int main(int argc, char const **argv)
{
    if (argc != 3)
    {
        std::cerr << "Bad params. Usage: complextype_server <host> <port>" << std::endl;
        return -1;
    }
    try
    {
        using BoostSerializer = Mif::Remote::Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
        using BoostDeserializer = Mif::Remote::Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
        using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<BoostSerializer, BoostDeserializer>;

        auto serviceFactory = std::make_shared<Mif::Service::ServiceFactory>();
        serviceFactory->AddClass<MyCompanyService>("MyCompanyService");

        using StubClient = Mif::Remote::StubClient<SerializerTraits, IMyCompany_PS>;
        using StubFactory = Mif::Net::ClientFactory<ProtocolChain<StubClient>>;

        std::cout << "Starting server on \"" << argv[1] << ":" << argv[2] << "\" ..." << std::endl;

        auto netClientFactgory = std::make_shared<StubFactory>
            (
                Mif::Common::MakeCreator<StubClient>(serviceFactory)
            );

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
