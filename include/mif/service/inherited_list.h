//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_INHERITED_LIST_H__
#define __MIF_SERVICE_INHERITED_LIST_H__

// STD
#include <tuple>
#include <type_traits>

// MIF
#include "mif/common/detail/tuple_utility.h"
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Service
    {
        namespace Detail
        {
            template <typename T, typename ... TList>
            struct ExpandInterfaceBase
            {
                using List  = Common::Detail::MakeUniqueTuple
                        <
                            typename ExpandInterfaceBase
                            <
                                typename T::TBaseTypeTuple,
                                TList ...
                            >::List
                        >;
            };

            template <typename ... TInterface, typename ... TList>
            struct ExpandInterfaceBase<std::tuple<TInterface ... >, TList ... >
            {
                using List = Common::Detail::MakeUniqueTuple
                        <
                            typename Common::Detail::TupleCat
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
