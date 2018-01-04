//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/tcp_service_client.h>
#include <mif/common/log.h>

// COMMON
#include "common/id/service.h"
#include "common/ps/iface.h"

class Application
    : public Mif::Application::TcpServiceClient
{
public:
    using TcpServiceClient::TcpServiceClient;

private:
    // Mif.Application.TcpServiceClient
    virtual void Init(Mif::Service::IFactoryPtr factory) override final
    {
        auto service = factory->Create<Service::IEmployee>(Service::Id::Service);

        auto human = service->Cast<Service::IHuman>();
        MIF_LOG(Info) << "Name: \"" << human->GetName() << "\"";
        MIF_LOG(Info) << "Age: \"" << human->GetAge() << "\"";

        service->Planning();
        service->Development();
        service->BugFixes();
        service->Monitoring();

        MIF_LOG(Info) << "Rate: \"" << service->GetRate() << "\"";
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
