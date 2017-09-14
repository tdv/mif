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
#include "mif/common/static_string.h"
#include "mif/orm/schema.h"
#include "mif/reflection/reflection.h"

namespace Mif
{
    namespace Orm
    {
        namespace PostgreSql
        {
            namespace Detail
            {

                MIF_DECLARE_SRTING_PROVIDER(DefailtSchemaName, "public")

                template <typename TName, typename ... TItems>
                class Schema final
                {
                public:
                    static std::string Dump()
                    {
                        std::string schemaName;
                        std::string sql = CreateSchema<TName>(schemaName);
                        return sql;
                    }

                private:
                    template <typename T>
                    static typename std::enable_if<std::is_same<T, DefailtSchemaName>::value, std::string>::type
                    CreateSchema(std::string &name)
                    {
                        name = std::string{};
                        return {};
                    }

                    template <typename T>
                    static typename std::enable_if<!std::is_same<T, DefailtSchemaName>::value, std::string>::type
                    CreateSchema(std::string &name)
                    {
                        name = T::GetString();
                        std::string sql;
                        sql = "CREATE SCHEMA " + name + ";\n\n";
                        name += ".";
                        return sql;
                    }
                };

            }   // namespace Detail

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
