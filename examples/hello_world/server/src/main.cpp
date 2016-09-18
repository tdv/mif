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
#include "common/ps/ihello_world.h"

// THIS
#include "hello_world_service.h"

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

        auto serviceFactory = std::make_shared<Mif::Service::ServiceFactory>();
        serviceFactory->AddClass<HelloWorldService>("HelloWorld");

        using StubClient = Mif::Remote::StubClient<SerializerTraits, IHelloWorld_PS>;
        using StubFactory = Mif::Net::ClientFactory<StubClient>;

        std::cout << "Starting server on \"" << argv[1] << ":" << argv[2] << "\" ..." << std::endl;

        auto netClientFactgory = std::make_shared<StubFactory>(
            std::static_pointer_cast<Mif::Service::IServiceFactory>(serviceFactory));
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