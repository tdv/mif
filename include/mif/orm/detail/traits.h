//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_DETAIL_TRAITS_H__
#define __MIF_ORM_DETAIL_TRAITS_H__

// STD
#include <cstdint>
#include <tuple>
#include <type_traits>

// MIF
#include "mif/common/detail/tuple_utility.h"
#include "mif/common/unused.h"
#include "mif/orm/detail/entity_info.h"
#include "mif/orm/detail/field_traits.h"

namespace Mif
{
    namespace Orm
    {
        namespace Detail
        {
            namespace Traits
            {
                namespace Detail
                {

                    template <typename T>
                    constexpr std::true_type IsEnum(EnumInfo<T> const *);
                    constexpr std::false_type IsEnum(...);

                    template <typename TEntity, typename ... TTraits>
                    constexpr std::true_type IsTable(TableInfo<TEntity, TTraits ... > const *);
                    constexpr std::false_type IsTable(...);

                    template <typename TField, typename TTraits>
                    struct FieldTraitsList;

                    template <typename T, std::size_t I, typename TPk>
                    struct PrimaryKeyCoFields
                        : public PrimaryKeyCoFields<T, I - 1, typename std::tuple_element<I - 1, T>::type>
                    {
                    };

                    template <typename T, std::size_t I, typename ... TCoFields>
                    struct PrimaryKeyCoFields<T, I, Orm::Detail::FieldTraits::PrimaryKey<TCoFields ... >>
                        : public std::true_type
                    {
                        using CoFields = std::tuple<TCoFields ... >;
                    };

                    template <typename T, typename TPk>
                    struct PrimaryKeyCoFields<T, 0, TPk>
                        : public std::false_type
                    {
                        using CoFields = std::tuple<>;
                    };

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
                inline constexpr bool IsEnum()
                {
                    return std::is_same<decltype(Detail::IsEnum(static_cast<T const *>(nullptr))), std::true_type>::value;
                }

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

                template <typename TTraits>
                using PrimaryKeyTrait = Detail::PrimaryKeyCoFields
                    <
                        TTraits,
                        std::tuple_size<TTraits>::value,
                        Orm::Detail::FieldTraits::No
                    >;

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


                namespace Detail
                {

                    template <typename TFields, std::size_t I, typename TTraits, typename TRes>
                    struct PrimaryKeyFields
                    {
                        using Field = typename TFields::template Field<I - 1>;
                        using Traits = MakeFieldTraitsList<Field, TTraits>;

                        using PrimaryKey = PrimaryKeyTrait<Traits>;

                        static_assert(std::tuple_size<TRes>::value == 0,
                                "Mif::Orm::Detail::Traits::Detail::PrimaryKeyFields "
                                "Each table must have only one primary key."
                            );

                        using Fields = typename PrimaryKeyFields
                            <
                                TFields,
                                I - 1,
                                TTraits,
                                typename std::conditional
                                    <
                                        PrimaryKey::value,
                                        typename Common::Detail::TupleCat
                                            <
                                                typename Common::Detail::TupleCat<std::tuple<Field>, TRes>::Tuple,
                                                typename PrimaryKey::CoFields
                                            >::Tuple,
                                        TRes
                                    >::type
                            >::Fields;
                    };

                    template <typename TFields, typename TTraits, typename TRes>
                    struct PrimaryKeyFields<TFields, 0, TTraits, TRes>
                    {
                        using Fields = TRes;
                    };

                }   // namespace Detail

                template <typename TFields, typename TTraits>
                using PrimaryKeyFields = typename Detail::PrimaryKeyFields
                    <
                        TFields,
                        TFields::Count,
                        TTraits,
                        std::tuple<>
                    >::Fields;

            }   // namespace Traits
        }   // namespace Detail
    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_SCHEMA_H__
