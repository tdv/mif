//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/application.h>
#include <mif/common/log.h>
#include <mif/net/http_clients.h>

// COMMON
#include "common/client.h"
#include "common/ps/iadmin.h"

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

        std::chrono::microseconds const timeout{10 * 1000 * 1000};

        auto clientFactory = Service::Ipc::MakeWebClientFactory(timeout);

        Mif::Net::HTTPClients clients(clientFactory);

        auto proxy = std::static_pointer_cast<Service::Ipc::ClientsChain>(clients.RunClient(host, port, "/admin"));

        MIF_LOG(Info) << "Client is successfully started.";

        auto client = proxy->GetClientItem<Service::Ipc::WebPSClient>();

        auto service = client->CreateService<Service::IAdmin>("WebService.Admin");

        MIF_LOG(Info) << "Client started.";


        MIF_LOG(Info) << "Add words.";

        service->SetTitle("Client title");
        service->SetBody("Client body");

        MIF_LOG(Info) << "Page preview: \n" << service->GetPage();
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Applicatin>(argc, argv);
}
