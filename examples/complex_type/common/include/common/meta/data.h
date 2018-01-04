//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __COMPLEX_TYPE_COMMON_META_DATA_H__
#define __COMPLEX_TYPE_COMMON_META_DATA_H__

// MIF
#include <mif/reflection/reflect_type.h>

// THIS
#include "common/data/data.h"

namespace Service
{
    namespace Data
    {
        namespace Meta
        {

            MIF_REFLECT_BEGIN(Human)
                MIF_REFLECT_FIELD(name)
                MIF_REFLECT_FIELD(lastName)
                MIF_REFLECT_FIELD(age)
            MIF_REFLECT_END()

            MIF_REFLECT_BEGIN(Position)
                MIF_REFLECT_FIELD(Unknown)
                MIF_REFLECT_FIELD(Developer)
                MIF_REFLECT_FIELD(Manager)
            MIF_REFLECT_END()

            MIF_REFLECT_BEGIN(Employee, Human)
                MIF_REFLECT_FIELD(position)
            MIF_REFLECT_END()

        }   // namespace Meta
    }   // namespace Data
}   // namespace Service

MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Meta::Human)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Meta::Position)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Meta::Employee)

#endif  // !__COMPLEX_TYPE_COMMON_META_DATA_H__
