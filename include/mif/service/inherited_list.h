//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_INHERITED_LIST_H__
#define __MIF_SERVICE_INHERITED_LIST_H__

// STD
#include <tuple>
#include <type_traits>

// MIF
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Service
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

            template<typename ... >
            struct Disjunction
                : public std::false_type
            {
            };

            template<typename B1>
            struct Disjunction<B1>
                : public B1
            {
            };

            template<typename B1, typename ... Bn>
            struct Disjunction<B1, Bn ... >
                : public std::conditional<B1::value, B1, Disjunction<Bn ... >>::type
            {
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

            template <typename T, typename ... TList>
            struct ExpandInterfaceBase
            {
                using List  = MakeUniqueTuple<typename ExpandInterfaceBase<typename T::TBaseTypeTuple, TList ... >::List>;
            };

            template <typename ... TInterface, typename ... TList>
            struct ExpandInterfaceBase<std::tuple<TInterface ... >, TList ... >
            {
                using List = MakeUniqueTuple
                        <
                            typename TupleCat
                            <
                                std::tuple<TInterface ... , TList ... >,
                                typename ExpandInterfaceBase<TInterface>::List ...
                            >::Tuple
                        >;
            };

            template <typename ... TList>
            struct ExpandInterfaceBase<IService, TList ... >
            {
                using List  = std::tuple<>;
            };

            template <typename ... TList>
            struct ExpandInterfaceBase<std::tuple<IService>, TList ... >
            {
                using List = std::tuple<>;
            };

        }   // namespace Detail

        template <typename T>
        using MakeInheritedIist = typename Detail::ExpandInterfaceBase
                <
                    typename std::enable_if
                    <
                        std::is_base_of<IService, T>::value || std::is_same<IService, T>::value,
                        T
                    >::type
                >::List;

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_INHERITED_LIST_H__
