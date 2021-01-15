//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_CLIENTS_FRAME_READER_H__
#define __MIF_NET_CLIENTS_FRAME_READER_H__

// STD
#include <memory>

// MIF
#include "mif/net/client.h"

namespace Mif
{
    namespace Net
    {
        namespace Clients
        {

            class FrameReader final
                : public Net::Client
            {
            public:
                FrameReader(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher);
                ~FrameReader();

            private:
                class Impl;
                std::unique_ptr<Impl> m_impl;

                // Client
                virtual void ProcessData(Common::Buffer buffer) override final;
            };

        }   // namespace Clients
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_CLIENTS_FRAME_READER_H__
