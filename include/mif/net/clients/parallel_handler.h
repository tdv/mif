//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_CLIENTS_PARALLEL_HANDLER_H__
#define __MIF_NET_CLIENTS_PARALLEL_HANDLER_H__

// STD
#include <memory>

// MIF
#include "mif/common/thread_pool.h"
#include "mif/net/iclient_factory.h"
#include "mif/net/client.h"

namespace Mif
{
    namespace Net
    {
        namespace Clients
        {

            class ParallelHandler final
                : public Net::Client
            {
            public:
                ParallelHandler(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher,
                        Common::IThreadPoolPtr workers);
                ~ParallelHandler();

            private:
                class Impl;
                std::shared_ptr<Impl> m_impl;

                // Client
                virtual void ProcessData(Common::Buffer buffer) override final;
            };

        }   // namespace Clients
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_CLIENTS_PARALLEL_HANDLER_H__
