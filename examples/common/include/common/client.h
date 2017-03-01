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

// BOOST
//#include <boost/archive/xml_iarchive.hpp>
//#include <boost/archive/xml_oarchive.hpp>

// MIF
#include <mif/common/thread_pool.h>
#include <mif/net/clients_chain.h>
#include <mif/net/client_factory.h>
#include <mif/net/clients/parallel_handler.h>
#include <mif/net/clients/frame_reader.h>
#include <mif/net/clients/frame_writer.h>
#include <mif/net/clients/gzip_compressor.h>
#include <mif/net/clients/gzip_decompressor.h>
#include <mif/remote/ps_client.h>
#include <mif/remote/serialization/serialization.h>
//#include <mif/remote/serialization/boost.h>
#include <mif/remote/serialization/json.h>
#include <mif/service/ifactory.h>
#include <mif/service/factory.h>
#include <mif/service/make.h>

namespace Service
{
    namespace Ipc
    {
        namespace Detail
        {

            using Serializer = Mif::Remote::Serialization::Json::Serializer;
            using Deserializer = Mif::Remote::Serialization::Json::Deserializer;
            using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<Serializer, Deserializer>;

        }   // namespace Detail

        using PSClient = Mif::Remote::PSClient<Detail::SerializerTraits>;

        namespace Detail
        {

            using ProtocolChain = Mif::Net::ClientsChain
                <
                    //Mif::Net::Clients::FrameReader,
                    //Mif::Net::Clients::ParallelHandler,
                    //Mif::Net::Clients::GZipDecompressor,
                    PSClient//,
                    //Mif::Net::Clients::GZipCompressor,
                    //Mif::Net::Clients::FrameWriter
                >;

            using ClientFactory = Mif::Net::ClientFactory<ProtocolChain>;

        }   // namespace Detail

        using ClientsChain = Detail::ProtocolChain;

        inline Mif::Net::IClientFactoryPtr MakeClientFactory(std::uint16_t threadCount,
                std::chrono::microseconds const &timeout,
                Mif::Service::IFactoryPtr factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::IFactory>())
        {
            auto workers = Mif::Common::CreateThreadPool(threadCount);

            return std::make_shared<Detail::ClientFactory>
                (
                    Mif::Common::MakeCreator<Mif::Net::Clients::ParallelHandler>(workers),
                    Mif::Common::MakeCreator<PSClient>(timeout, factory)
                );
        }

    }   // namespace Ipc
}   // namespace Service

#endif  // !__EXAMPLES_COMMON_CLIENT_H__
