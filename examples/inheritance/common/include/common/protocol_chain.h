//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __INHERITANCE_COMMON_PROTOCOL_CHAIN_H__
#define __INHERITANCE_COMMON_PROTOCOL_CHAIN_H__

// STD
#include <string>

// MIF
#include <mif/remote/ps.h>

// THIS
#include <mif/net/clients_chain.h>
#include <mif/net/clients/frame_reader.h>
#include <mif/net/clients/frame_writer.h>
#include <mif/net/clients/gzip_compressor.h>
#include <mif/net/clients/gzip_decompressor.h>

namespace Service
{
    namespace Ipc
    {

        template <typename T>
        using ProtocolChain = Mif::Net::ClientsChain
            <
                Mif::Net::Clients::FrameReader,
                Mif::Net::Clients::GZipDecompressor,
                T,
                Mif::Net::Clients::GZipCompressor,
                Mif::Net::Clients::FrameWriter
            >;

    }   // namespace Ipc
}   // namespace Service

#endif  // !__INHERITANCE_COMMON_PROTOCOL_CHAIN_H__
