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

                using DefailtSchemaName = MIF_STATIC_STR("");

            }   // namespace StringProviders

            template <typename ... T>
            struct Items
            {
            };

            template <typename TEntity, typename ... TTraits>
            struct TableInfo
            {
                using Type = TEntity;
            };

            struct FieldTraits
            {
                struct Counter;
                struct NotNull;
                struct Nullable;
                template <typename ... TCoFields>
                struct PrimaryKey;
                struct Unique;
            };

            template <typename TTable, typename TField, typename ... TTraits>
            struct FieldInfo
            {
                template <typename TNextField>
                using Field = FieldInfo<TTable, TNextField, TTraits ... >;

                using Create = typename TTable::template CreateTable<TTraits ... >;

                using Counter = FieldInfo<TTable, TField, FieldTraits::Counter, TTraits ... >;
                using NotNull = FieldInfo<TTable, TField, FieldTraits::NotNull, TTraits ... >;
                using Nullable = FieldInfo<TTable, TField, FieldTraits::Nullable, TTraits ... >;
                template <typename ... TCoFields>
                using MultiPrimaryKey = FieldInfo<TTable, TField, FieldTraits::PrimaryKey<TCoFields ... >, TTraits ... >;
                using PrimaryKey = MultiPrimaryKey<>;
                using Unique = FieldInfo<TTable, TField, FieldTraits::Unique, TTraits ... >;
            };

        }   // namespace Detail

        using DefailtSchemaName = Detail::StringProviders::DefailtSchemaName;

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

        template <typename TEntity>
        class Table
        {
        public:
            static_assert(Reflection::IsReflectable<TEntity>(), "[Mif::Orm::Table] The type must be a reflectable type.");

            using ThisType = Table<TEntity>;

            using Create = Detail::TableInfo<TEntity>;

            template <typename TField, typename ... TTraits>
            using Field = Detail::FieldInfo<ThisType, TField, TTraits ... >;

        private:
            template <typename, typename, typename ... >
            friend struct Detail::FieldInfo;

            template <typename ... TTraits>
            using CreateTable = Detail::TableInfo<TEntity, TTraits ... >;
        };

    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_SCHEMA_H__
