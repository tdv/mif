//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_DETAIL_TABLE_TRAITS_H__
#define __MIF_ORM_DETAIL_TABLE_TRAITS_H__

namespace Mif
{
    namespace Orm
    {
        namespace Detail
        {

            struct TableTraits
            {
                struct OnlyInfo;

                struct Temporary;
                struct WithoutLogging;
            };

        }   // namespace Detail
    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_DETAIL_TABLE_TRAITS_H__
