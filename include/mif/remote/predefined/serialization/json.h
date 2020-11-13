//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_PREDEFINED_SERIALIZATION_JSON_H__
#define __MIF_REMOTE_PREDEFINED_SERIALIZATION_JSON_H__

// MIF
#include "mif/remote/serialization/json.h"
#include "mif/remote/serialization/serialization.h"

namespace Mif
{
    namespace Remote
    {
        namespace Predefined
        {
            namespace Serialization
            {

                using Json = Remote::Serialization::SerializerTraits
                        <
                            Remote::Serialization::Json::Serializer,
                            Remote::Serialization::Json::Deserializer
                        >;

            }   // namespace Serialization
        }   // namespace Predefined
    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_PREDEFINED_SERIALIZATION_JSON_H__
