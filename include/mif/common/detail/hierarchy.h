//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_DETAIL_HIERARCHY_H__
#define __MIF_COMMON_DETAIL_HIERARCHY_H__

// STD
#include <cstdint>
#include <type_traits>

namespace Mif
{
    namespace Common
    {
        namespace Detail
        {

            template <std::size_t N>
            struct Hierarchy
                : public Hierarchy<N - 1>
            {
            };

            template <>
            struct Hierarchy<0>
            {
            };

            template <std::size_t N>
            using MakeHierarchy = Hierarchy<N>;

            using FakeHierarchyLength = std::integral_constant<std::size_t, 100>;
            using FakeHierarchy = MakeHierarchy<FakeHierarchyLength::value>;

        }   // namespace Detail

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_DETAIL_HIERARCHY_H__
