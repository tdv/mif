//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_POSTGRESQL_DETAIL_UTILITY_H__
#define __MIF_ORM_POSTGRESQL_DETAIL_UTILITY_H__

// STD
#include <string>

namespace Mif
{
    namespace Orm
    {
        namespace PostgreSql
        {
            namespace Detail
            {
                namespace Utility
                {

                    std::string QuoteReserved(std::string const &str);

                }   // namespace Utility
            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Orm

#endif  // !__MIF_ORM_POSTGRESQL_DETAIL_UTILITY_H__
