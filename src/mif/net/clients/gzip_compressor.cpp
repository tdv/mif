//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

// STD
#include <utility>

// BOOST
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/stream.hpp>

// MIF
#include "mif/net/clients/gzip_compressor.h"

namespace Mif
{
    namespace Net
    {
        namespace Clients
        {

            GZipCompressor::GZipCompressor(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher)
                : Client(control, publisher)
            {
            }

            void GZipCompressor::ProcessData(Common::Buffer buffer)
            {
                Common::Buffer result;

                {
                    using SourceType = boost::iostreams::basic_array_source<char>;
                    SourceType source{buffer.data(), buffer.size()};
                    boost::iostreams::stream<SourceType> iStream{source};

                    boost::iostreams::filtering_ostream oStream;
                    oStream.push(boost::iostreams::gzip_compressor{});
                    oStream.push(boost::iostreams::back_inserter(result));

                    boost::iostreams::copy(iStream, oStream);
                }

                Post(std::move(result));
            }

        }   // namespace Clients
    }   // namespace Net
}   // namespace Mif
