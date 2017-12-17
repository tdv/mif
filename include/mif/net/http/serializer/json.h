//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_SERIALIZER_JSON_H__
#define __MIF_NET_HTTP_SERIALIZER_JSON_H__

// MIF
#include "mif/serialization/json.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Serializer
            {

                struct Json final
                {
                    static constexpr char const* GetContentType()
                    {
                        return "application/json; charset=UTF-8";
                    }

                    template <typename T>
                    static Common::Buffer Serialize(T const &data)
                    {
                        return Serialization::Json::Serialize(data);
                    }
                };

            }   // namespace Serializer
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif


#endif  // !__MIF_NET_HTTP_SERIALIZER_JSON_H__
