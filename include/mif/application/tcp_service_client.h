//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_APPLICATION_TCP_SERVICE_CLIENT_H__
#define __MIF_APPLICATION_TCP_SERVICE_CLIENT_H__

// STD
#include <chrono>
#include <cstdint>
#include <functional>

// MIF
#include "mif/application/net_base_application.h"
#include "mif/service/ifactory.h"
#include "mif/remote/predefined/client_factory.h"

namespace Mif
{
    namespace Application
    {

        class TcpServiceClient
            : public NetBaseApplication
        {
        public:
            using ClientFactory = std::function<
                    Net::IClientFactoryPtr (std::uint16_t /*workers*/,
                            std::chrono::microseconds const & /*timeout*/,
                            Service::FactoryPtr /*factory*/)
                >;

            using ServiceCreator = std::function<
                    Service::IServicePtr (Net::IClientFactory::ClientPtr /*client*/,
                            Service::ServiceId /*service id*/)
                >;

            TcpServiceClient(int argc, char const **argv,
                    ClientFactory const &clientFactory = Remote::Predefined::MakeClientFactory<>,
                    ServiceCreator const &serviceCreator = Remote::Predefined::CreateService<>);

        protected:
            virtual void Init(Service::IFactoryPtr factory);
            virtual void Done();

        private:
            ClientFactory m_clientFactory;
            ServiceCreator m_serviceCreator;

            // NetBaseApplication
            virtual void OnInit() override final;
            virtual void OnDone() override final;
        };

    }   // namespace Application
}   // namespace Mif

#endif  // !__MIF_APPLICATION_TCP_SERVICE_CLIENT_H__
