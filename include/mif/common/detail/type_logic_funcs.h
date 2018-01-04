//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_DETAIL_TYPE_LOGIC_FUNCS_H__
#define __MIF_COMMON_DETAIL_TYPE_LOGIC_FUNCS_H__

// STD
#include <type_traits>

namespace Mif
{
    namespace Common
    {
        namespace Detail
        {

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

        }   // namespace Detail
    }  // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_DETAIL_TYPE_LOGIC_FUNCS_H__
