//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/tcp_service.h>

// COMMON
#include "common/id/service.h"
#include "common/ps/ihello_world.h"

class Application
    : public Mif::Application::TcpService
{
public:
    using TcpService::TcpService;

private:
    // Mif.Application.TcpService
    virtual void Init(Mif::Service::FactoryPtr factory) override final
    {
        factory->AddClass<Service::Id::HelloWorld>();
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
