//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_SCHEMA_H__
#define __MIF_ORM_SCHEMA_H__

// MIF
#include "mif/reflection/reflection.h"

namespace Mif
{
    namespace Orm
    {

        template <typename TName, typename ... TItems>
        struct Schema
        {
        };

        template <typename T, typename ... TOptions>
        struct Table
        {
            static_assert(Reflection::IsReflectable<T>(), "[Mif::Orm::Table] The type must be a reflectable type.");
        };

    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_SCHEMA_H__
