//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_SCHEMA_H__
#define __MIF_ORM_SCHEMA_H__

// STD
#include <string>
#include <tuple>
#include <type_traits>

// MIF
#include "mif/common/static_string.h"
#include "mif/orm/detail/entity_info.h"
#include "mif/orm/detail/field_traits.h"
#include "mif/orm/detail/traits.h"
#include "mif/reflection/reflection.h"

namespace Mif
{
    namespace Orm
    {

        using DefailtSchemaName = MIF_STATIC_STR("");

        template <typename T>
        class Enum
        {
        public:
            static_assert(std::is_enum<T>::value && Reflection::IsReflectable<T>(),
                    "[Mif::Orm::Table] The type must be a enum type and a reflectable type.");

            using Create = Detail::EnumInfo<T>;
        };

        template <typename TEntity>
        class Table
        {
        public:
            static_assert(Reflection::IsReflectable<TEntity>(), "[Mif::Orm::Table] The type must be a reflectable type.");
            static_assert(!std::tuple_size<typename Reflection::Reflect<TEntity>::Base>::value,
                    "[Mif::Orm::Table] The type must not have base types."
                );

            using EntityType = TEntity;
            using ThisType = Table<TEntity>;

            using Create = Detail::TableInfo<TEntity>;

            template <typename TField>
            using Field = Detail::FieldInfo<ThisType, TField, Detail::FieldTraits::No>;

        private:
            template <typename, typename, typename ... >
            friend struct Detail::FieldInfo;

            template <typename TTraits>
            using CreateTable = Detail::TableInfo<TEntity, TTraits>;
        };

        // TODO:
        // Add Partition by data(year, month or day), hash or count
        // Add Index and UniqueIndex by one or moere fields

        template <typename TName, typename ... TItems>
        struct Schema
        {
            using Name = TName;
            using Items = std::tuple<TItems ... >;

            template <template <typename ... > class T>
            static std::string CreateSchema()
            {
                return T<Schema<Name, TItems ... >>::CreateSchema();
            }
        };

    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_SCHEMA_H__
