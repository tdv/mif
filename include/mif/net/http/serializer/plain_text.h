//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_SERIALIZER_PLAIN_TEXT_H__
#define __MIF_NET_HTTP_SERIALIZER_PLAIN_TEXT_H__

// BOOST
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/stream.hpp>

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Serializer
            {

                struct PlainText final
                {
                    static constexpr char const* GetContentType()
                    {
                        return "text/html; charset=UTF-8";
                    }

                    template <typename T>
                    static Common::Buffer Serialize(T const &data)
                    {
                        Common::Buffer buffer;

                        {
                            boost::iostreams::filtering_ostream stream{boost::iostreams::back_inserter(buffer)};
                            stream << data;
                            stream.flush();
                        }

                        return buffer;
                    }
                };

            }   // namespace Serializer
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif


#endif  // !__MIF_NET_HTTP_SERIALIZER_PLAIN_TEXT_H__
