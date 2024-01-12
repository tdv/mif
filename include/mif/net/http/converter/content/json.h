//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_CONVERTER_CONTENT_JSON_H__
#define __MIF_NET_HTTP_CONVERTER_CONTENT_JSON_H__

// STD
#include <stdexcept>

// MIF
#include "mif/serialization/json.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Converter
            {
                namespace Content
                {

                    struct Json final
                    {
                        template <typename T>
                        static T Convert(Common::Buffer const &buffer)
                        {
                            if (buffer.empty())
                                throw std::invalid_argument{"[Mif::Net::Http::Converter::Content::Json] No content."};
                            return Serialization::Json::Deserialize<T>(buffer);
                        }
                    };

                }   // namespace Content
            }   // namespace Converter
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_CONVERTER_CONTENT_JSON_H__
