//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     04.2021
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_HASH_CALC_H__
#define __MIF_COMMON_HASH_CALC_H__

// STD
#include <cstdint>
#include <string>

namespace Mif
{
    namespace Common
    {
        namespace Hash
        {

            std::string Crc32(void const *data, std::size_t size);
            std::string Md5(void const *data, std::size_t size);
            std::string Sha1(void const *data, std::size_t size);

        }   // namespace Hash
    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_HASH_CALC_H__
