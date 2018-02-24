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
#include <string>
#include <type_traits>

// MIF
#include "mif/common/static_string.h"
#include "mif/orm/common.h"

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

                    template <typename TSchemaName, typename TEntityName>
                    struct EntityName final
                    {
                        static std::string Create()
                        {
                            using Schema = typename std::conditional
                                <
                                    std::is_same<TSchemaName, Orm::DefaultSchemaName>::value,
                                    DefaultSchemaName,
                                    TSchemaName
                                >::type;

                            using Delimiter = typename std::conditional
                                <
                                    std::is_same<TSchemaName, Orm::DefaultSchemaName>::value,
                                    MIF_STATIC_STR(""),
                                    MIF_STATIC_STR(".")
                                >::type;

                            std::string name = Schema::Value;
                            name += Delimiter::Value;
                            name += TEntityName::Value;
                            return Utility::QuoteReserved(Utility::PascalCaseToUnderlining(name));
                        }
                    };

                }   // namespace Utility
            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Orm

#endif  // !MIF_WITH_POSTGRESQL
#endif  // !__MIF_ORM_POSTGRESQL_DETAIL_UTILITY_H__
