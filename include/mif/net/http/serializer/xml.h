//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_SERIALIZER_XML_H__
#define __MIF_NET_HTTP_SERIALIZER_XML_H__

// STD
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
            namespace Serializer
            {

                struct Xml final
                {
                    static constexpr char const* GetContentType()
                    {
                        return "text/xml; charset=UTF-8";
                    }

                    template <typename T>
                    static typename std::enable_if<Reflection::IsReflectable<T>(), Common::Buffer>::type Serialize(T const &data)
                    {
                        return Serialization::Xml::Serialize(data);
                    }

                    template <typename T>
                    static typename std::enable_if<!Reflection::IsReflectable<T>(), Common::Buffer>::type Serialize(T const &data)
                    {
                        return Serialization::Xml::Serialize(data, Detail::Tag::Document::Value);
                    }
                };

            }   // namespace Serializer
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif


#endif  // !__MIF_NET_HTTP_SERIALIZER_XML_H__
