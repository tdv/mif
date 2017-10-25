//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_SCHEMA_H__
#define __MIF_ORM_SCHEMA_H__

// STD
#include <cstdint>
#include <tuple>

// MIF
#include "mif/common/detail/tuple_utility.h"
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

            template <typename TEntity, typename ... TTraits>
            struct TableInfo
            {
                using Type = TEntity;
                using Traits = std::tuple<TTraits ... >;
            };

            struct FieldTraits
            {
                struct No;

                struct Counter;
                struct JsonObject;
                struct NotNull;
                struct Nullable;
                template <typename ... TCoFields>
                struct PrimaryKey;
                struct Unique;
            };

            template <typename TTable, typename TField, typename TTrait, typename ... TNext>
            class FieldInfo
            {
            private:
                using ThisType = FieldInfo<TTable, TField, TTrait, TNext ... >;

            public:
                template <typename TOtherField>
                using Field = FieldInfo<TTable, TOtherField, FieldTraits::No, ThisType, TNext ... >;

                using Create = typename TTable::template CreateTable<ThisType>;

                using Counter = FieldInfo<TTable, TField, FieldTraits::Counter, ThisType, TNext ... >;
                using JsonObject = FieldInfo<TTable, TField, FieldTraits::JsonObject, ThisType, TNext ... >;
                using NotNull = FieldInfo<TTable, TField, FieldTraits::NotNull, ThisType, TNext ... >;
                using Nullable = FieldInfo<TTable, TField, FieldTraits::Nullable, ThisType, TNext ... >;
                template <typename ... TCoFields>
                using MultiPrimaryKey = FieldInfo<TTable, TField, FieldTraits::PrimaryKey<TCoFields ... >, ThisType, TNext ... >;
                using PrimaryKey = MultiPrimaryKey<>;
                using Unique = FieldInfo<TTable, TField, FieldTraits::Unique, ThisType, TNext ... >;

                // TODO:
                // Add AsJsonObject
            };

            namespace Traits
            {
                namespace Detail
                {

                    template <typename TEntity, typename ... TTraits>
                    constexpr std::true_type IsTable(TableInfo<TEntity, TTraits ... > const *);
                    constexpr std::false_type IsTable(...);

                    template <typename TField, typename TTraits>
                    struct FieldTraitsList;

                    template <typename TField>
                    struct FieldTraitsList<TField, std::tuple<>>
                    {
                        using Traits = std::tuple<FieldTraits::No>;
                    };

                    template
                    <
                        typename TField,
                        typename TTable,
                        typename TFieldMeta,
                        typename TTrait,
                        typename TNext,
                        typename ... TTail
                    >
                    struct FieldTraitsList<TField, FieldInfo<TTable, TFieldMeta, TTrait, TNext, TTail ... >>
                    {
                        using Traits = typename FieldTraitsList<TField, TNext>::Traits;
                    };

                    template
                    <
                        typename TField,
                        typename TTable,
                        typename TTrait,
                        typename TNext,
                        typename ... TTail
                    >
                    struct FieldTraitsList<TField, FieldInfo<TTable, TField, TTrait, TNext, TTail ... >>
                    {
                        using Traits = Common::Detail::MakeUniqueTuple
                            <
                                typename Common::Detail::TupleCat
                                <
                                    std::tuple<TTrait>,
                                    typename FieldTraitsList<TField, TNext>::Traits
                                >::Tuple
                            >;
                    };


                    template
                    <
                        typename TField,
                        typename TTable,
                        typename TFieldMeta,
                        typename TTrait
                    >
                    struct FieldTraitsList<TField, FieldInfo<TTable, TFieldMeta, TTrait>>
                    {
                        using Traits = std::tuple<TTrait>;
                    };

                    template <typename TTable, typename TField, typename TTrait, typename ... TNext>
                    constexpr std::true_type IsFieldInfo(FieldInfo<TTable, TField, TTrait, TNext ... > const *);
                    constexpr std::false_type IsFieldInfo(...);

                    template <typename TTraits, std::size_t I, typename TFieldTraits>
                    struct FindFieldsTraits
                    {
                        using Traits = typename std::conditional
                            <
                                std::is_same
                                    <
                                        std::true_type,
                                        decltype(IsFieldInfo(
                                                static_cast<typename std::tuple_element<I - 1, TTraits>::type const *>(nullptr)
                                            ))
                                    >::value,
                                typename std::tuple_element<I - 1, TTraits>::type,
                                typename FindFieldsTraits
                                    <
                                        TTraits,
                                        I - 1,
                                        typename std::tuple_element<I - 1, TTraits>::type
                                    >::Traits
                            >::type;
                    };

                    template <typename TTraits, typename TFieldTraits>
                    struct FindFieldsTraits<TTraits, 0, TFieldTraits>
                    {
                        using Traits = std::tuple<>;
                    };

                }   // namespace Detail

                template <typename T>
                inline constexpr bool IsTable()
                {
                    return std::is_same<decltype(Detail::IsTable(static_cast<T const *>(nullptr))), std::true_type>::value;
                }

                template <typename T>
                inline constexpr bool IsFieldInfo()
                {
                    return std::is_same<decltype(Detail::IsFieldInfo(static_cast<T const *>(nullptr))), std::true_type>::value;
                }

                template <typename TField, typename TTraits>
                using MakeFieldTraitsList = typename Detail::FieldTraitsList
                    <
                        TField,
                        typename Detail::FindFieldsTraits
                            <
                                TTraits,
                                std::tuple_size<TTraits>::value,
                                std::tuple<>
                            >::Traits
                    >::Traits;

            }   // namespace Traits
        }   // namespace Detail

        using DefailtSchemaName = Detail::StringProviders::DefailtSchemaName;

        template <typename TEntity>
        class Table
        {
        public:
            static_assert(Reflection::IsReflectable<TEntity>(), "[Mif::Orm::Table] The type must be a reflectable type.");

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
            static std::string Dump()
            {
                return T<Schema<Name, Items>>::Dump();
            }
        };

    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_SCHEMA_H__
