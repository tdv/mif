#ifndef __PHONEBOOK_DATA_META_DATA_H__
#define __PHONEBOOK_DATA_META_DATA_H__

// MIF
#include <mif/reflection/reflect_type.h>

// THIS
#include "data/data.h"

namespace Phonebook
{
    namespace Data
    {
        namespace Meta
        {

            MIF_REFLECT_BEGIN(Status)
                MIF_REFLECT_FIELD(code)
                MIF_REFLECT_FIELD(message)
            MIF_REFLECT_END()

            MIF_REFLECT_BEGIN(Info)
                MIF_REFLECT_FIELD(service)
                MIF_REFLECT_FIELD(node)
                MIF_REFLECT_FIELD(timestamp)
                MIF_REFLECT_FIELD(status)
            MIF_REFLECT_END()

            MIF_REFLECT_BEGIN(Header)
                MIF_REFLECT_FIELD(meta)
            MIF_REFLECT_END()

            MIF_REFLECT_BEGIN(Phones)
                MIF_REFLECT_FIELD(meta)
                MIF_REFLECT_FIELD(data)
            MIF_REFLECT_END()

        }   // namespace Meta
    }   // namespace Data
}   // namespace Phonebook

MIF_REGISTER_REFLECTED_TYPE(Phonebook::Data::Meta::Status)
MIF_REGISTER_REFLECTED_TYPE(Phonebook::Data::Meta::Info)
MIF_REGISTER_REFLECTED_TYPE(Phonebook::Data::Meta::Header)
MIF_REGISTER_REFLECTED_TYPE(Phonebook::Data::Meta::Phones)

#endif  // !__PHONEBOOK_DATA_META_DATA_H__
