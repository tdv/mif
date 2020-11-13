//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_TYPES_H__
#define __MIF_COMMON_TYPES_H__

// STD
#include <memory>
#include <vector>

namespace Mif
{
    namespace Common
    {

        using Buffer = std::vector<char>;
        using BufferPtr = std::shared_ptr<Buffer>;

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_TYPES_H__
