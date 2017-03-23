//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/application.h>
#include <mif/common/log.h>
#include <mif/net/tcp_clients.h>

// COMMON
#include "common/client.h"
#include "common/ps/ihello_world.h"

class Applicatin
    : public Mif::Application::Application
{
public:
    Applicatin(int argc, char const **argv)
        : Mif::Application::Application{argc, argv}
    {
        boost::program_options::options_description options{"Client options"};
        options.add_options()
                ("host", boost::program_options::value<std::string>()->default_value("0.0.0.0"), "Server host")
                ("port", boost::program_options::value<std::string>()->default_value("55555"), "Server port");

        AddCustomOptions(options);
    }

private:
    // Mif.Application.Application
    virtual void OnStart() override final
    {
        auto const &options = GetOptions();

        auto const host = options["host"].as<std::string>();
        auto const port = options["port"].as<std::string>();

        MIF_LOG(Info) << "Starting client on " << host << ":" << port;

        std::chrono::microseconds timeout{10 * 1000 * 1000};

        auto clientFactory = Service::Ipc::MakeClientFactory(4, timeout);

        Mif::Net::TCPClients clients(clientFactory);

        auto proxy = std::static_pointer_cast<Service::Ipc::ClientsChain>(clients.RunClient(host, port));

        MIF_LOG(Info) << "Client is successfully started.";

        auto client = proxy->GetClientItem<Service::Ipc::PSClient>();

        auto service = client->CreateService<Service::IHelloWorld>("HelloWorld");

        MIF_LOG(Info) << "Client started.";

        MIF_LOG(Info) << "Add words.";

        service->AddWord("Hello");
        service->AddWord("World");
        service->AddWord("!!!");

        MIF_LOG(Info) << "Result from server: \"" << service->GetText() << "\"";

        MIF_LOG(Info) << "Clean.";
        service->Clean();

        MIF_LOG(Info) << "Result from server: \"" << service->GetText() << "\"";
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Applicatin>(argc, argv);
}
