//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_DETAIL_UTILITY_H__
#define __MIF_ORM_DETAIL_UTILITY_H__

// MIF
#include "mif/common/static_string.h"

namespace Mif
{
    namespace Orm
    {
        namespace Detail
        {
            namespace Utility
            {

                template <typename T>
                using CreateUniqueName = T;

            }   // namespace Utility
        }   // namespace Detail
    }   // namespace Orm
}   // namespace Orm

#endif  // !__MIF_ORM_DETAIL_UTILITY_H__
