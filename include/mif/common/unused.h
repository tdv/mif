//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_UNUSED_H__
#define __MIF_COMMON_UNUSED_H__

// STD
#include <vector>

namespace Mif
{
    namespace Common
    {

        template <typename ... T>
        inline void Unused(T && ... )
        {
        }

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_UNUSED_H__
