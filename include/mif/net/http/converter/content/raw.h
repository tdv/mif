//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_CONVERTER_CONTENT_RAW_H__
#define __MIF_NET_HTTP_CONVERTER_CONTENT_RAW_H__

// STD
#include <stdexcept>
#include <type_traits>

// MIF
#include "mif/common/types.h"
#include "mif/serialization/traits.h"

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

                    struct Raw final
                    {
                        template <typename T>
                        static typename std::enable_if
                                <
                                    Serialization::Traits::IsIterable<T>() ||
                                        std::is_same<T, std::string>::value,
                                    T
                                >::type
                        Convert(Common::Buffer const &buffer)
                        {
                            if (buffer.empty())
                                throw std::invalid_argument{"[Mif::Net::Http::Converter::Content::Raw] No content."};
                            return {std::begin(buffer), std::end(buffer)};
                        }
                    };

                }   // namespace Content
            }   // namespace Converter
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_CONVERTER_CONTENT_RAW_H__
