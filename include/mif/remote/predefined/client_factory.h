//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_PREDEFINED_CLIENT_FACTORY_H__
#define __MIF_REMOTE_PREDEFINED_CLIENT_FACTORY_H__

// STD
#include <chrono>
#include <cstdint>
#include <memory>

// MIF
#include "mif/common/thread_pool.h"
#include "mif/net/client_factory.h"
#include "mif/remote/ps_client.h"
#include "mif/remote/predefined/protocol/archived_frame.h"
#include "mif/remote/predefined/serialization/boost/binary.h"
#include "mif/service/ifactory.h"
#include "mif/service/factory.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Remote
    {
        namespace Predefined
        {

            template <typename TSerialization = Serialization::Boost::Binary>
            inline Net::IClientFactoryPtr MakeClientFactory(std::uint16_t threadCount,
                    std::chrono::microseconds const &timeout,
                    Service::IFactoryPtr factory = Service::Make<Service::Factory, Service::IFactory>())
            {
                auto workers = Common::CreateThreadPool(threadCount);

                using Client = PSClient<TSerialization>;
                using ProtocolChain = Protocol::ArchivedFrame<Client>;
                using ClientFactory = Net::ClientFactory<ProtocolChain>;

                return std::make_shared<ClientFactory>
                    (
                        Common::MakeCreator<Net::Clients::ParallelHandler>(workers),
                        Common::MakeCreator<Client>(timeout, factory)
                    );
            }

        }   // namespace Predefined
    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_PREDEFINED_CLIENT_FACTORY_H__
