//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MICROSERVICES_COMMON_DATA_META_DATA_H__
#define __MICROSERVICES_COMMON_DATA_META_DATA_H__

// MIF
#include <mif/reflection/reflect_type.h>

// THIS
#include "common/data/data.h"

namespace Common
{
    namespace Data
    {
        namespace Meta
        {

            MIF_REFLECT_BEGIN(Position)
                MIF_REFLECT_FIELD(Unknown)
                MIF_REFLECT_FIELD(Developer)
                MIF_REFLECT_FIELD(Manager)
            MIF_REFLECT_END()

            MIF_REFLECT_BEGIN(Employee)
                MIF_REFLECT_FIELD(name)
                MIF_REFLECT_FIELD(lastName)
                MIF_REFLECT_FIELD(age)
                MIF_REFLECT_FIELD(email)
                MIF_REFLECT_FIELD(position)
                MIF_REFLECT_FIELD(rate)
            MIF_REFLECT_END()

        }   // namespace Meta
    }   // namespace Data
}   // namespace Common

MIF_REGISTER_REFLECTED_TYPE(::Common::Data::Meta::Position)
MIF_REGISTER_REFLECTED_TYPE(::Common::Data::Meta::Employee)

#endif  // !__MICROSERVICES_COMMON_DATA_META_DATA_H__
