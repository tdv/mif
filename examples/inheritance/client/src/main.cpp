//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2017 tdv
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
#include "common/ps/iface.h"
#include "common/protocol_chain.h"

int main(int argc, char const **argv)
{
    if (argc != 3)
    {
        std::cerr << "Bad params. Usage: protostack_client <host> <port>" << std::endl;
        return -1;
    }
    try
    {
        using BoostSerializer = Mif::Remote::Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
        using BoostDeserializer = Mif::Remote::Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
        using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<BoostSerializer, BoostDeserializer>;

        using ProxyClient = Mif::Remote::ProxyClient<SerializerTraits>;

        using ClientsChain = Service::Ipc::ProtocolChain<ProxyClient>;

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

        auto service = client->CreateService<Service::IEmployee>("Service");

        std::cout << "Client started." << std::endl;

        std::cout << "Name: \"" << service->GetName() << "\"" << std::endl;
        std::cout << "Age: \"" << service->GetAge() << "\"" << std::endl;

        service->Planning();
        service->Development();
        service->BugFixes();
        service->Monitoring();

        std::cout << "Wage: \"" << service->GetWages() << "\"" << std::endl;

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
