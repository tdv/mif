//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_POSTGRESQL_DRIVER_H__
#define __MIF_ORM_POSTGRESQL_DRIVER_H__

// MIF
#include "mif/common/config.h"
#ifdef MIF_WITH_POSTGRESQL

// BOOST
#include <boost/algorithm/string.hpp>

// STD
#include <string>
#include <tuple>

// MIF
#include "mif/orm/detail/entity.h"
#include "mif/orm/forward.h"
#include "mif/orm/postgresql/detail/common.h"
#include "mif/orm/postgresql/detail/modifications/inserter.h"
#include "mif/orm/postgresql/detail/structure.h"

namespace Mif
{
    namespace Orm
    {
        namespace PostgreSql
        {

            template <typename TSchema>
            class Driver
            {
            public:
                static std::string CreateSchema()
                {
                    Detail::StringList items;
                    Detail::Entity<std::tuple<>, SchemaType>::Create(items);
                    return boost::algorithm::join(items, "\n");
                }

                template <typename T>
                static std::string Insert(T const &value)
                {
                    Detail::StringList items;
                    Detail::Inserter<SchemaType>::Insert(value, items);
                    return boost::algorithm::join(items, "\n");
                }

            private:
                using SchemaType = TSchema;
            };

        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Mif

#endif  // !MIF_WITH_POSTGRESQL
#endif  // !__MIF_ORM_POSTGRESQL_DRIVER_H__
