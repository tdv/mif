#ifndef __MIF_COMMON_HIERARCHY_H__
#define __MIF_COMMON_HIERARCHY_H__

// STD
#include <cstdint>

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

        }   // namespace Detail

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_HIERARCHY_H__
