//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_SERIALIZATION_SERIALIZATION_H__
#define __MIF_REMOTE_SERIALIZATION_SERIALIZATION_H__

namespace Mif
{
    namespace Remote
    {
        namespace Serialization
        {

            template <typename TSerializer, typename TDeserializer>
            class SerializerTraits final
            {
            public:
                using Serializer = TSerializer;
                using Deserializer = TDeserializer;
            };

        }   // namespace Serialization
    }   //  namespace Remote
}   // namespace Mif


#endif  // !__MIF_REMOTE_SERIALIZATION_SERIALIZATION_H__
