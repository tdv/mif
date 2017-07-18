//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __EXAMPLES_COMMON_CLIENT_H__
#define __EXAMPLES_COMMON_CLIENT_H__

// STD
#include <chrono>
#include <cstdint>
#include <memory>

// MIF
#include <mif/net/client_factory.h>
#include <mif/remote/ps_client.h>
#include <mif/remote/predefined/serialization/json.h>
#include <mif/remote/predefined/client_factory.h>
#include <mif/service/ifactory.h>
#include <mif/service/factory.h>
#include <mif/service/make.h>

namespace Service
{
    namespace Ipc
    {

        using WebPSClient = Mif::Remote::PSClient<Mif::Remote::Predefined::Serialization::Json>;

        namespace Detail
        {

            using WebProtocolChain = Mif::Net::ClientsChain<WebPSClient>;
            using WebClientFactory = Mif::Net::ClientFactory<WebProtocolChain>;

        }   // namespace Detail

        using WebClientsChain = Detail::WebProtocolChain;

        inline Mif::Net::IClientFactoryPtr MakeWebClientFactory(std::chrono::microseconds const &timeout,
                Mif::Service::IFactoryPtr factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::IFactory>())
        {
            return std::make_shared<Detail::WebClientFactory>
                (
                    Mif::Common::MakeCreator<WebPSClient>(timeout, factory)
                );
        }

    }   // namespace Ipc
}   // namespace Service

#endif  // !__EXAMPLES_COMMON_CLIENT_H__
