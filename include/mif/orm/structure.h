//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_STRUCTURE_H__
#define __MIF_ORM_STRUCTURE_H__

// STD
#include <tuple>
#include <type_traits>

// MIF
#include "mif/common/static_string.h"
#include "mif/orm/detail/entity.h"
#include "mif/orm/detail/table_fields.h"

namespace Mif
{
    namespace Orm
    {

        using DefaultSchemaName = MIF_STATIC_STR("");

        template <typename>
        class Table;

        template <typename, typename ... >
        class Schema;

        template <char const ... TName, typename ... TEntities>
        class Schema<Common::StaticString<TName ... >, Detail::Entity<TEntities> ... > final
        {
        public:
            using ThisType = Schema<Common::StaticString<TName ... >, Detail::Entity<TEntities> ... >;
            using Create = Detail::Entity<ThisType>;

        private:
        };

        template <typename T>
        class Enum final
        {
        public:
            using ThisType = Enum<T>;
            using Create = Detail::Entity<ThisType>;

        private:
            static_assert(std::is_enum<T>::value && Reflection::IsReflectable<T>(),
                    "[Mif::Orm::Enum] The type must be a reflectable enum.");
        };

        template <typename T>
        class Table final
        {
        public:
            using EntityType = T;
            using ThisType = Table<T>;
            using Create = Detail::Entity<ThisType>;

            template <typename TFieldMeta>
            using Field = Detail::FieldInfo<ThisType, TFieldMeta, std::tuple<>,
                    Detail::FieldTraits::AvailableTraits<typename std::decay<typename TFieldMeta::Type>::type>,
                    std::tuple<>, std::tuple<>>;

        private:
            static_assert(std::is_class<T>::value && Reflection::IsReflectable<T>(),
                    "[Mif::Orm::Table] The type must be a reflectable struct.");
            static_assert(std::tuple_size<typename Reflection::Reflect<T>::Base>::value == 0,
                    "[Mif::Orm::Table] The type should not have any basic structs.");
        };

        // TODO: support index and partitional

    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_STRUCTURE_H__
