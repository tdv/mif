//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/net_base_application.h>
#include <mif/common/crc32.h>
#include <mif/common/log.h>
#include <mif/net/http/clients.h>

// THIS
#include "common/client.h"
#include "common/id/service.h"
#include "common/ps/iadmin.h"

class Application
    : public Mif::Application::NetBaseApplication
{
public:
    using NetBaseApplication::NetBaseApplication;

private:
    // Mif.Application.Application
    virtual void OnInit() override final
    {
        auto clientFactory = Service::Ipc::MakeClientFactory(GetTimeout());

        Mif::Net::Http::Clients clients(clientFactory);

        auto proxy = std::static_pointer_cast<Service::Ipc::ClientsChain>(clients.RunClient(
                GetHost(), GetPort(), "/admin"));

        auto client = proxy->GetClientItem<Service::Ipc::PSClient>();

        auto service = client->CreateService<Service::IAdmin>(Service::Id::Service);

        MIF_LOG(Info) << "Set new document title and body.";

        service->SetTitle("Client title");
        service->SetBody("Client body");

        MIF_LOG(Info) << "Page preview: \n" << service->GetPage();
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
