//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_APPLICATION_TCP_SERVICE_H__
#define __MIF_APPLICATION_TCP_SERVICE_H__

// STD
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>

// MIF
#include "mif/application/net_base_application.h"
#include "mif/service/factory.h"
#include "mif/net/tcp/server.h"
#include "mif/remote/predefined/utility.h"

namespace Mif
{
    namespace Application
    {

        class TcpService
            : public NetBaseApplication
        {
        public:
            using ClientFactory = std::function<
                    Net::IClientFactoryPtr (std::uint16_t /*workers*/,
                            std::chrono::microseconds const & /*timeout*/,
                            Service::FactoryPtr /*factory*/)
                >;

            TcpService(int argc, char const **argv,
                    ClientFactory const &clientFactory = Remote::Predefined::MakeClientFactory<>);

        protected:
            virtual void Init(Service::FactoryPtr factory);
            virtual void Done();

        private:
            ClientFactory m_clientFactory;
            std::unique_ptr<Net::Tcp::Server> m_server;

            // NetBaseApplication
            virtual void OnInit() override final;
            virtual void OnDone() override final;
        };

    }   // namespace Application
}   // namespace Mif

#endif  // !__MIF_APPLICATION_TCP_SERVICE_H__
