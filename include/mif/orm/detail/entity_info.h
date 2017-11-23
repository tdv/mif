//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_DETAIL_ENTITY_INFO_H__
#define __MIF_ORM_DETAIL_ENTITY_INFO_H__

// STD
#include <tuple>

// MIF
#include "mif/orm/detail/field_traits.h"
#include "mif/orm/detail/table_traits.h"

namespace Mif
{
    namespace Orm
    {
        namespace Detail
        {
            template <typename T>
            struct EnumInfo
            {
                using Type = T;
            };

            template <typename TEntity, typename ... TTraits>
            struct TableInfo
            {
                using Type = TEntity;
                using Traits = std::tuple<TTraits ... >;

                using OnlyInfo = TableInfo<TEntity, TTraits ... , TableTraits::OnlyInfo>;
                using Temporary = TableInfo<TEntity, TTraits ... , TableTraits::Temporary>;
                using WithoutLogging = TableInfo<TEntity, TTraits ... , TableTraits::WithoutLogging>;
            };

            template <typename TTable, typename TField, typename TTrait, typename ... TNext>
            class FieldInfo
            {
            private:
                using ThisType = FieldInfo<TTable, TField, TTrait, TNext ... >;

                static_assert(std::is_same<typename TTable::EntityType, typename TField::Class>::value,
                        "[Mif::Orm::Detail::FieldInfo] Field must be a member of the entity type."
                    );

            public:
                template <typename TOtherField>
                using Field = FieldInfo<TTable, TOtherField, FieldTraits::No, ThisType, TNext ... >;

                using Create = typename TTable::template CreateTable<ThisType>;

                using Counter = FieldInfo<TTable, TField, FieldTraits::Counter, ThisType, TNext ... >;
                using NotNull = FieldInfo<TTable, TField, FieldTraits::NotNull, ThisType, TNext ... >;
                using Nullable = FieldInfo<TTable, TField, FieldTraits::Nullable, ThisType, TNext ... >;
                template <typename ... TCoFields>
                using MultiPrimaryKey = FieldInfo<TTable, TField, FieldTraits::PrimaryKey<TCoFields ... >, ThisType, TNext ... >;
                using PrimaryKey = MultiPrimaryKey<>;
                using Unique = FieldInfo<TTable, TField, FieldTraits::Unique, ThisType, TNext ... >;
                using WithTimezone = FieldInfo<TTable, TField, FieldTraits::WithTimezone, ThisType, TNext ... >;
            };

        }   // namespace Detail
    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_DETAIL_ENTITY_INFO_H__
