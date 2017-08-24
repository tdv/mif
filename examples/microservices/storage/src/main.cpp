//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// MIF
#include <mif/application/tcp_service.h>
#include <mif/service/create.h>

// COMMON
#include "common/id/service.h"
//#include "common/interface/ps/iemployee_storage.h"

namespace Storage
{

    class Application
        : public Mif::Application::TcpService
    {
    public:
        using TcpService::TcpService;

    private:
        // Mif.Application.Application
        virtual void Init(Mif::Service::FactoryPtr factory) override final
        {
            auto config = GetConfig();
            if (!config)
                throw std::runtime_error{"[Storage::Application::Init] No config."};

            auto dbConfig = config->GetConfig("database");
            if (!dbConfig)
                throw std::runtime_error{"[Storage::Application::Init] No 'database' config node."};

            auto storage = Mif::Service::Create<Common::Service::Id::PGEmployeeStorage>(dbConfig);

            factory->AddInstance(Common::Service::Id::PGEmployeeStorage, storage);
        }
    };

}   // namespace Storage

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Storage::Application>(argc, argv);
}
