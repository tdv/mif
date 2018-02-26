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
#include "mif/orm/forward.h"
#include "mif/orm/detail/entity.h"
#include "mif/orm/detail/table_fields.h"

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

        }   // namespace Detail
    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_DETAIL_TRAITS_UTILITY_H__
