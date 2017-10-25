//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_DETAIL_FIELD_TRAITS_H__
#define __MIF_ORM_DETAIL_FIELD_TRAITS_H__

namespace Mif
{
    namespace Orm
    {
        namespace Detail
        {

            struct FieldTraits
            {
                struct No;

                struct Counter;
                struct NotNull;
                struct Nullable;
                template <typename ... TCoFields>
                struct PrimaryKey;
                struct Unique;
            };

        }   // namespace Detail
    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_DETAIL_FIELD_TRAITS_H__
