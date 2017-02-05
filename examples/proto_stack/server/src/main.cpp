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
#include "common/ps/iface.h"

// MIF
#include <mif/remote/ps_client.h>

// THIS
#include "common/id/service.h"
#include "common/protocol_chain.h"

int main(int argc, char const **argv)
{
    if (argc != 3)
    {
        std::cerr << "Bad params. Usage: protostack_server <host> <port>" << std::endl;
        return -1;
    }
    try
    {
        using BoostSerializer = Mif::Remote::Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
        using BoostDeserializer = Mif::Remote::Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
        using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<BoostSerializer, BoostDeserializer>;

        auto factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::Factory>();
        factory->AddClass<::Service::Id::Service>();

        using PSClient = Mif::Remote::PSClient<SerializerTraits>;
        using StubFactory = Mif::Net::ClientFactory<Service::Ipc::ProtocolChain<PSClient>>;

        std::cout << "Starting server on \"" << argv[1] << ":" << argv[2] << "\" ..." << std::endl;

        std::chrono::microseconds timeout{10 * 1000 * 1000};

        auto netClientFactgory = std::make_shared<StubFactory>
            (
                Mif::Common::MakeCreator<PSClient>(timeout, factory)
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
