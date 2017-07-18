//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/tcp_service.h>

// COMMON
#include "common/id/service.h"
#include "common/ps/iface.h"

class Application
    : public Mif::Application::TcpService
{
public:
    using TcpService::TcpService;

private:
    // Mif.Application.TcpService
    virtual void Init(Mif::Service::FactoryPtr factory) override final
    {
        factory->AddClass<::Service::Id::Service>();
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
