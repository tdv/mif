//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __HTTP_COMMON_CLIENT_H__
#define __HTTP_COMMON_CLIENT_H__

// STD
#include <chrono>
#include <cstdint>
#include <memory>

// MIF
#include <mif/net/clients_chain.h>
#include <mif/net/client_factory.h>
#include <mif/remote/ps_client.h>
#include <mif/remote/predefined/serialization/json.h>
#include <mif/service/ifactory.h>
#include <mif/service/factory.h>
#include <mif/service/make.h>

namespace Service
{
    namespace Ipc
    {

        using PSClient = Mif::Remote::PSClient<Mif::Remote::Predefined::Serialization::Json>;

        namespace Detail
        {

            using ProtocolChain = Mif::Net::ClientsChain<PSClient>;
            using ClientFactory = Mif::Net::ClientFactory<ProtocolChain>;

        }   // namespace Detail

        using ClientsChain = Detail::ProtocolChain;

        inline Mif::Net::IClientFactoryPtr MakeClientFactory(std::chrono::microseconds const &timeout,
                Mif::Service::IFactoryPtr factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::IFactory>())
        {
            return std::make_shared<Detail::ClientFactory>
                (
                    Mif::Common::MakeCreator<PSClient>(timeout, factory)
                );
        }

    }   // namespace Ipc
}   // namespace Service

#endif  // !__HTTP_COMMON_CLIENT_H__
