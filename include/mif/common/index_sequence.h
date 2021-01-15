//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_INDEX_SEQUENCE_H__
#define __MIF_COMMON_INDEX_SEQUENCE_H__

// STD
#include <cstdint>

namespace Mif
{
    namespace Common
    {

        template <std::size_t ... Indexes>
        struct IndexSequence
        {
        };

        namespace Detail
        {

            template <std::size_t N, std::size_t ... Indexes>
            struct IndexSequenceGenerator
                : IndexSequenceGenerator<N - 1, N - 1, Indexes ... >
            {
            };

            template <std::size_t ... Indexes>
            struct IndexSequenceGenerator<0u, Indexes ... >
            {
                using Type = IndexSequence<Indexes ... >;
            };

        }   // namespace Detail

        template <std::size_t N>
        using MakeIndexSequence = typename Detail::IndexSequenceGenerator<N>::Type;

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_INDEX_SEQUENCE_H__
