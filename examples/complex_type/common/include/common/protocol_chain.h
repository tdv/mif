//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __COMPLEX_TYPE_COMMON_PROTOCOL_CHAIN_H__
#define __COMPLEX_TYPE_COMMON_PROTOCOL_CHAIN_H__

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

#endif  // !__COMPLEX_TYPE_COMMON_PROTOCOL_CHAIN_H__
