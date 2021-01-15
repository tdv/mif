//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/tcp_service_client.h>
#include <mif/common/log.h>

// COMMON
#include "common/id/service.h"
#include "common/ps/ihello_world.h"

class Application
    : public Mif::Application::TcpServiceClient
{
public:
    using TcpServiceClient::TcpServiceClient;

private:
    // Mif.Application.TcpServiceClient
    virtual void Init(Mif::Service::IFactoryPtr factory) override final
    {
        auto service = factory->Create<Service::IHelloWorld>(Service::Id::HelloWorld);

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
    return Mif::Application::Run<Application>(argc, argv);
}
