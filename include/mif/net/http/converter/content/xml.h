//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_CONVERTER_CONTENT_XML_H__
#define __MIF_NET_HTTP_CONVERTER_CONTENT_XML_H__

// STD
#include <stdexcept>
#include <type_traits>

// MIF
#include "mif/net/http/detail/content_tags.h"
#include "mif/reflection/reflection.h"
#include "mif/serialization/xml.h"

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

                    struct Xml final
                    {
                        template <typename T>
                        static typename std::enable_if<Reflection::IsReflectable<T>(), T>::type Convert(Common::Buffer const &buffer)
                        {
                            if (buffer.empty())
                                throw std::invalid_argument{"[Mif::Net::Http::Converter::Content::Xml] No content."};
                            return Serialization::Xml::Deserialize<T>(buffer);
                        }

                        template <typename T>
                        static typename std::enable_if<!Reflection::IsReflectable<T>(), T>::type Convert(Common::Buffer const &buffer)
                        {
                            if (buffer.empty())
                                throw std::invalid_argument{"[Mif::Net::Http::XmlContentParamConverter] No content."};
                            return Serialization::Xml::Deserialize<T>(buffer, Detail::Tag::Document::Value);
                        }
                    };

                }   // namespace Content
            }   // namespace Converter
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_CONVERTER_CONTENT_XML_H__
