//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>
#include <mutex>

// BOOST
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

// MIF
#include <mif/net/client_factory.h>
#include <mif/net/tcp_clients.h>
#include <mif/remote/serialization/serialization.h>
#include <mif/remote/serialization/boost.h>
#include <mif/service/make.h>

// COMMON
#include "common/ps/iface.h"
#include "common/protocol_chain.h"

// MIF
#include <mif/remote/ps_client.h>

namespace Service
{
    namespace Detail
    {
        namespace
        {

        class MessageVisitor
            : public Mif::Service::Inherit<IMessageVisitor>
        {
        private:
            using LockType = std::mutex;
            using LockGuard = std::lock_guard<LockType>;

            LockType m_lock;

            // IMessageVisitor
            virtual void Visit(IMessagePtr message) override final
            {
                if (!message)
                    throw std::invalid_argument{"Empty message pointer."};

                LockGuard lock{m_lock};
                std::cout << "Message: " << message->GetText() << std::endl;
            }
        };

        }   // namespace
    }   // namespace Detail
}   // namespace Service

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

        using PSClient = Mif::Remote::PSClient<SerializerTraits>;

        using ClientsChain = Service::Ipc::ProtocolChain<PSClient>;

        using ProxyFactory = Mif::Net::ClientFactory<ClientsChain>;

        std::chrono::microseconds timeout{10 * 1000 * 1000};

        std::cout << "Starting client on \"" << argv[1] << ":" << argv[2] << "\"" << std::endl;

        auto clientFactgory = std::make_shared<ProxyFactory>
            (
                Mif::Common::MakeCreator<PSClient>(timeout)
            );

        Mif::Net::TCPClients clients(4, clientFactgory);

        auto proxy = std::static_pointer_cast<ClientsChain>(clients.RunClient(argv[1], argv[2]));

        auto client = proxy->GetClientItem<PSClient>();

        auto service = client->CreateService<Service::IViewer>("Service");

        std::cout << "Client started." << std::endl;

        auto visitor = Mif::Service::Make<Service::Detail::MessageVisitor, Service::IMessageVisitor>();
        service->Accept(visitor);

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
