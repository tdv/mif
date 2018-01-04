//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_PREDEFINED_PROTOCOL_SIMPLE_FRAME_H__
#define __MIF_REMOTE_PREDEFINED_PROTOCOL_SIMPLE_FRAME_H__

// MIF
#include "mif/net/clients_chain.h"
#include "mif/net/clients/parallel_handler.h"
#include "mif/net/clients/frame_reader.h"
#include "mif/net/clients/frame_writer.h"

namespace Mif
{
    namespace Remote
    {
        namespace Predefined
        {
            namespace Protocol
            {

                template <typename TClient>
                using SimpleFrame = Net::ClientsChain
                        <
                            Net::Clients::FrameReader,
                            Net::Clients::ParallelHandler,
                            TClient,
                            Net::Clients::FrameWriter
                        >;

            }   // namespace Protocol
        }   // namespace Predefined
    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_PREDEFINED_PROTOCOL_SIMPLE_FRAME_H__
