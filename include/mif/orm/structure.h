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
#include "mif/orm/common.h"
#include "mif/orm/detail/entity.h"
#include "mif/orm/detail/table_fields.h"
#include "mif/orm/forward.h"

namespace Mif
{
    namespace Orm
    {

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

        template <typename T, typename ... TTraits>
        class Table final
        {
        private:
            template <typename ... TTableTraits>
            using CreateTable = Detail::Entity<Table<T, TTraits ... , TTableTraits ... >>;

        public:
            using EntityType = T;
            using ThisType = Table<T, TTraits ... >;
            using Create = CreateTable<>;

            template <typename TFieldMeta>
            using Field = Detail::FieldInfo<ThisType, TFieldMeta, std::tuple<>,
                    Detail::FieldTraits::AvailableTraits<typename std::decay<typename TFieldMeta::Type>::type>,
                    std::tuple<>, std::tuple<>>;

        private:
            static_assert(std::is_class<T>::value && Reflection::IsReflectable<T>(),
                    "[Mif::Orm::Table] The type must be a reflectable struct.");
            static_assert(std::tuple_size<typename Reflection::Reflect<T>::Base>::value == 0,
                    "[Mif::Orm::Table] The type should not have any basic structs.");

            template <typename, typename, typename, typename, typename, typename>
            friend class Detail::FieldInfo;
        };

        // TODO: support index and partitional

    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_STRUCTURE_H__
