#ifndef __MIF_COMMON_TYPES_H__
#define __MIF_COMMON_TYPES_H__

// STD
#include <cstdint>

// BOOST
#include <boost/shared_array.hpp>

namespace Mif
{
    namespace Common
    {

        using CharArray = boost::shared_array<char>;
        using Buffer = std::pair<std::size_t, CharArray>;

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_TYPES_H__
