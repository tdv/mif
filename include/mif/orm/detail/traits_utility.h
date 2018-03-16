//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_DETAIL_TRAITS_UTILITY_H__
#define __MIF_ORM_DETAIL_TRAITS_UTILITY_H__

// STD
#include <cstdint>
#include <tuple>
#include <type_traits>

// MIF
#include "mif/common/detail/tuple_utility.h"
#include "mif/orm/forward.h"
#include "mif/orm/detail/entity.h"
#include "mif/orm/detail/table_fields.h"
#include "mif/reflection/reflection.h"

namespace Mif
{
    namespace Orm
    {
        namespace Detail
        {
            template <typename, typename ... >
            struct SelectFieldTraits;

            template <typename TFieldMeta, typename TFieldTraits, typename ... TTraits>
            struct FindFieldTraits;

            template <typename TFieldMeta, typename TTrait, typename ... TFieldTraits, typename ... TTraits>
            struct FindFieldTraits<TFieldMeta, std::tuple<TTrait, TFieldTraits ... >, TTraits ... > final
            {
                using Traits = typename FindFieldTraits<TFieldMeta, std::tuple<TFieldTraits ... >, TTraits ... >::Traits;
            };

            template <typename TFieldMeta, typename TFieldTraits, typename ... TTraits>
            struct FindFieldTraits<TFieldMeta, std::tuple<FieldDescr<TFieldMeta, TFieldTraits>, TTraits ... >> final
            {
                using Traits = TFieldTraits;
            };

            template <typename TFieldMeta, typename TTrait, typename ... TTraits>
            struct SelectFieldTraits<TFieldMeta, TTrait, TTraits ... > final
            {
                using Traits = typename FindFieldTraits<TFieldMeta, TTrait, TTraits ... >::Traits;
            };

            template <typename TFieldMeta>
            struct SelectFieldTraits<TFieldMeta> final
            {
                using Traits = std::tuple<>;
            };

            template <typename TFieldMeta, typename ... TTraits>
            struct FindFieldTraits<TFieldMeta, std::tuple<>, TTraits ... > final
            {
                using Traits = typename SelectFieldTraits<TFieldMeta, TTraits ... >::Traits;
            };

            template <typename TTable, typename TFieldMeta>
            struct FieldTraitsList final
            {
                using Traits = std::tuple<>;
            };

            template <typename TFieldMeta, typename TEntity, typename ... TTraits>
            struct FieldTraitsList<Entity<Table<TEntity, TTraits ... >>, TFieldMeta> final
            {
                using Traits = typename SelectFieldTraits<TFieldMeta, TTraits ... >::Traits;
            };

            template <std::uint32_t TraitId, typename TTraits>
            struct HasTrait;

            template <std::uint32_t TraitId, typename TTrait, typename ... TTraits>
            struct HasTrait<TraitId, std::tuple<TTrait, TTraits ... >>
                : public HasTrait<TraitId, std::tuple<TTraits ... >>
            {
            };

            template <std::uint32_t TraitId, typename ... TTraits>
            struct HasTrait<TraitId, std::tuple<FieldTraits::FieldTrait<TraitId>, TTraits ... >>
                : public std::true_type
            {
            };

            template <std::uint32_t TraitId>
            struct HasTrait<TraitId, std::tuple<>>
                : public std::false_type
            {
            };

            template <std::uint32_t TraitId, typename TTable, typename TFields, std::size_t N>
            class FieldsWithTrait final
            {
            private:
                using Field = typename TFields::template Field<N - 1>;
                using FieldTraits = typename FieldTraitsList<TTable, Field>::Traits;

            public:
                using Fields = typename std::conditional
                    <
                        HasTrait<TraitId, FieldTraits>::value,
                        Common::Detail::MakeUniqueTuple
                        <
                            typename Common::Detail::TupleCat
                            <
                                std::tuple<Field>,
                                typename FieldsWithTrait<TraitId, TTable, TFields, N - 1>::Fields
                            >::Tuple
                        >,
                        typename FieldsWithTrait<TraitId, TTable, TFields, N - 1>::Fields
                    >::type;
            };

            template <std::uint32_t TraitId, typename TTable, typename TFields>
            class FieldsWithTrait<TraitId, TTable, TFields, 0> final
            {
            public:
                using Fields = std::tuple<>;
            };

            template
            <
                typename TFieldType,
                typename TName,
                typename TOwner
            >
            class FakeField final
            {
            private:
                using ThisType = FakeField<TFieldType, TName, TOwner>;

            public:
                using FieldType = TFieldType;
                using TypeNameProvider = TName;
                using ClassType = TOwner;

                static constexpr FieldType ThisType::* Access()
                {
                    return &ThisType::m_field;
                }

            private:
                FieldType m_field;
            };

            template <typename>
            class PrimaryKey;

            template <typename TEntity, typename ... TTraits>
            class PrimaryKey<Entity<Table<TEntity, TTraits ... >>> final
            {
            private:
                using EntityFields = typename Reflection::Reflect<TEntity>::Fields;
                using EntityTable = Entity<Table<TEntity, TTraits ... >>;
                using PrimaryKeyFields = typename FieldsWithTrait
                <
                    FieldTraits::Trait_PrimaryKey,
                    EntityTable,
                    EntityFields,
                    EntityFields::Count
                >::Fields;
                using FakePrimaryKey = Reflection::Detail::FieldItem
                    <
                        EntityFields::Count,
                        FakeField
                        <
                            std::size_t,
                            Common::StringCat
                            <
                                MIF_STATIC_STR("pk_"),
                                Common::StringCat
                                <
                                    typename Reflection::Reflect<TEntity>::Name,
                                    MIF_STATIC_STR("_id")
                                >
                            >,
                            TEntity
                        >
                    >;

            public:
                using Fields = typename std::conditional
                    <
                        std::tuple_size<PrimaryKeyFields>::value,
                        PrimaryKeyFields,
                        std::tuple<FakePrimaryKey>
                    >::type;
            };

            template <typename TFields, std::size_t I = TFields::Count>
            struct HasFieldReference
                : public std::conditional
                    <
                        Reflection::IsReflectable<typename TFields::template Field<I - 1>::Type>(),
                        std::true_type,
                        HasFieldReference<TFields, I - 1>
                    >::type
            {
            };

            template <typename TFields>
            struct HasFieldReference<TFields, 0>
                : public std::false_type
            {
            };

            template <typename T, typename TEntities, std::size_t I = std::tuple_size<TEntities>::value>
            struct FindEntityByType
            {
                using Entity = typename std::conditional
                    <
                        std::is_same<T, typename std::tuple_element<I - 1, TEntities>::type::EntityType>::value,
                        typename std::tuple_element<I - 1, TEntities>::type,
                        typename FindEntityByType<T, TEntities, I - 1>::Entity
                    >::type;
            };

            template <typename T, typename TEntities>
            struct FindEntityByType<T, TEntities, 0>
            {
                using Entity = struct EntityNotFound;
            };

        }   // namespace Detail
    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_DETAIL_TRAITS_UTILITY_H__
