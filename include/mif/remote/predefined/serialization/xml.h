//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_PREDEFINED_SERIALIZATION_XML_H__
#define __MIF_REMOTE_PREDEFINED_SERIALIZATION_XML_H__

// MIF
#include "mif/remote/serialization/xml.h"
#include "mif/remote/serialization/serialization.h"

namespace Mif
{
    namespace Remote
    {
        namespace Predefined
        {
            namespace Serialization
            {

                using Xml = Remote::Serialization::SerializerTraits
                        <
                            Remote::Serialization::Xml::Serializer,
                            Remote::Serialization::Xml::Deserializer
                        >;

            }   // namespace Serialization
        }   // namespace Predefined
    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_PREDEFINED_SERIALIZATION_XML_H__
