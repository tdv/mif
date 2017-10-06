//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_DETAIL_TUPLE_UTILITY_H__
#define __MIF_COMMON_DETAIL_TUPLE_UTILITY_H__

// STD
#include <tuple>
#include <type_traits>

// MIF
#include "mif/common/detail/type_logic_funcs.h"

namespace Mif
{
    namespace Common
    {
        namespace Detail
        {

            template <typename H, typename ... T>
            struct TupleCat
            {
                using Tuple = typename TupleCat<H, typename TupleCat<T ... >::Tuple>::Tuple;
            };

            template <>
            struct TupleCat<std::tuple<>>
            {
                using Tuple = std::tuple<>;
            };

            template <typename ... X>
            struct TupleCat<std::tuple<X ... >>
            {
                using Tuple = std::tuple<X ... >;
            };

            template <typename ... X, typename ... Y>
            struct TupleCat<std::tuple<X ... >, std::tuple<Y ... >>
            {
                using Tuple = std::tuple<X ... , Y ... >;
            };

            template <typename T, typename Tuple>
            struct TupleContains;

            template <typename T, typename ... Types>
            struct TupleContains<T, std::tuple<Types ... >>
                : public Disjunction<std::is_same<T, Types> ... >
            {
            };

            template <typename, typename>
            struct UniqueTuple;

            template <typename H, typename ... T, typename ... TRes>
            struct UniqueTuple<std::tuple<H, T ... >, std::tuple<TRes ... >>
            {
                using Tuple = typename UniqueTuple
                        <
                            std::tuple<T ... >,
                            typename std::conditional
                            <
                                TupleContains<H, std::tuple<TRes ... >>::value,
                                std::tuple<TRes ... >,
                                std::tuple<H, TRes ... >
                            >::type
                        >::Tuple;
            };

            template <typename ... TRes>
            struct UniqueTuple<std::tuple<>, std::tuple<TRes ... >>
            {
                using Tuple = std::tuple<TRes ... >;
            };

            template <typename T>
            using MakeUniqueTuple = typename UniqueTuple<T, std::tuple<>>::Tuple;

        }   // namespace Detail
    }  // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_DETAIL_TUPLE_UTILITY_H__
