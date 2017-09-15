//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_SCHEMA_H__
#define __MIF_ORM_SCHEMA_H__

// MIF
#include "mif/common/static_string.h"
#include "mif/reflection/reflection.h"

namespace Mif
{
    namespace Orm
    {
        namespace Detail
        {
            namespace StringProviders
            {

                MIF_DECLARE_SRTING_PROVIDER(DefailtSchemaName, "")

            }   // namespace StringProviders

            template <typename ... T>
            struct Items
            {
            };

        }   // namespace Detail

        using DefailtSchemaName = Common::MakeStaticString<Detail::StringProviders::DefailtSchemaName>;

        template <typename TName, typename ... TItems>
        struct Schema
        {
            using Name = TName;
            using Items = Detail::Items<TItems ... >;

            template <template <typename ... > class T>
            static std::string Dump()
            {
                return T<Schema<TName, TItems ... >>::Dump();
            }
        };

        template <typename T, typename ... TOptions>
        struct Table
        {
            static_assert(Reflection::IsReflectable<T>(), "[Mif::Orm::Table] The type must be a reflectable type.");

            using Type = T;
        };

    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_SCHEMA_H__
