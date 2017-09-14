//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_POSTGRESQL_SCHEMA_H__
#define __MIF_ORM_POSTGRESQL_SCHEMA_H__

// STD
#include <string>

// MIF
#include "mif/orm/schema.h"
#include "mif/orm/postgresql/detail/schema.h"

namespace Mif
{
    namespace Orm
    {
        namespace PostgreSql
        {

            using DefailtSchemaName = Detail::DefailtSchemaName;

            template <typename TName, typename ... TItems>
            class Schema
                : public Orm::Schema<TName, TItems ... >
            {
            public:
                static std::string Dump()
                {
                    return Detail::Schema<TName, TItems ... >::Dump();
                }
            };

        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_POSTGRESQL_SCHEMA_H__
