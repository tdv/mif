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
#include <mif/remote/serialization/boost/serialization.h>

// COMMON
#include "common/ps/ihello_world.h"

int main(int argc, char const **argv)
{
    if (argc != 3)
    {
        std::cerr << "Bad params. Usage: helloworld_client <host> <port>" << std::endl;
        return -1;
    }
    try
    {
        using BoostSerializer = Mif::Remote::Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
        using BoostDeserializer = Mif::Remote::Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
        using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<BoostSerializer, BoostDeserializer>;

        using ProxyClient = Mif::Remote::ProxyClient<SerializerTraits, IHelloWorld_PS>;
        using ProxyFactory = Mif::Net::ClientFactory<ProxyClient>;

        std::chrono::microseconds timeout{10 * 1000 * 1000};

        std::cout << "Starting client on \"" << argv[1] << ":" << argv[2] << "\"" << std::endl;

        auto clientFactgory = std::make_shared<ProxyFactory>(timeout);
        Mif::Net::TCPClients clients(4, clientFactgory);

        auto client = std::static_pointer_cast<ProxyClient>(clients.RunClient(argv[1], argv[2]));

        auto helloWorldService = client->CreateService<IHelloWorld>("HelloWorld");

        std::cout << "Client started." << std::endl;

        std::cout << "Add words." << std::endl;

        helloWorldService->AddWord("Hello");
        helloWorldService->AddWord("World");
        helloWorldService->AddWord("!!!");

        std::cout << "Result from server: \"" << helloWorldService->GetText() << "\"" << std::endl;
        
        std::cout << "Clean." << std::endl;
        helloWorldService->Clean();
        
        std::cout << "Result from server: \"" << helloWorldService->GetText() << "\"" << std::endl;

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