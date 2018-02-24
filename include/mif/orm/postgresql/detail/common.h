//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_POSTGRESQL_DETAIL_COMMON_H__
#define __MIF_ORM_POSTGRESQL_DETAIL_COMMON_H__

// MIF
#include "mif/common/config.h"
#ifdef MIF_WITH_POSTGRESQL

// STD
#include <list>
#include <string>

// MIF
#include "mif/common/static_string.h"

namespace Mif
{
    namespace Orm
    {
        namespace PostgreSql
        {
            namespace Detail
            {

                using DefaultSchemaName = MIF_STATIC_STR("");
                using Indent = MIF_STATIC_STR("    ");

                using StringList = std::list<std::string>;

            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Orm

#endif  // !MIF_WITH_POSTGRESQL
#endif  // !__MIF_ORM_POSTGRESQL_DETAIL_COMMON_H__
