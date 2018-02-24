//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_POSTGRESQL_DETAIL_UTILITY_H__
#define __MIF_ORM_POSTGRESQL_DETAIL_UTILITY_H__

// MIF
#include "mif/common/config.h"
#ifdef MIF_WITH_POSTGRESQL

// STD
#include <set>
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
                    std::string Quote(std::string const &str);
                    std::string PascalCaseToUnderlining(std::string const &str);

                }   // namespace Utility
            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Orm

#endif  // !MIF_WITH_POSTGRESQL
#endif  // !__MIF_ORM_POSTGRESQL_DETAIL_UTILITY_H__
