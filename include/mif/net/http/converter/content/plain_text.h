//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_CONVERTER_CONTENT_PLAIN_TEXT_H__
#define __MIF_NET_HTTP_CONVERTER_CONTENT_PLAIN_TEXT_H__

// STD
#include <stdexcept>
#include <string>

// MIF
#include "mif/common/types.h"
#include "mif/net/http/converter/url/param.h"

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

                    struct PlainText final
                    {
                        template <typename T>
                        static T Convert(Common::Buffer const &buffer)
                        {
                            if (buffer.empty())
                                throw std::invalid_argument{"[Mif::Net::Http::Converter::Content::PlainText] No content."};
                            return Converter::Url::Param::template Convert<T>(std::string{std::begin(buffer), std::end(buffer)});
                        }
                    };

                }   // namespace Content
            }   // namespace Converter
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif


#endif  // !__MIF_NET_HTTP_CONVERTER_CONTENT_PLAIN_TEXT_H__
