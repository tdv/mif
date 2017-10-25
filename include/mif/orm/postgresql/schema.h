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
#include <tuple>
#include <type_traits>

// MIF
#include "mif/common/detail/tuple_utility.h"
#include "mif/common/unused.h"
#include "mif/orm/postgresql/detail/type_holder.h"
#include "mif/orm/postgresql/detail/utility.h"
#include "mif/orm/schema.h"
#include "mif/serialization/traits.h"

namespace Mif
{
    namespace Orm
    {
        namespace PostgreSql
        {

            template <typename T>
            class Schema;

            template <typename TName, typename ... TItems>
            class Schema<Orm::Schema<TName, std::tuple<TItems ... >>> final
            {
            public:
                static std::string Dump()
                {
                    std::string schema;

                    auto sql = CreateSchema<typename SourceSchema::Name>(schema);

                    if (!sql.empty())
                        sql += "\n";

                    sql += CreateItem<Items, std::tuple_size<Items>::value>(schema);

                    return sql;
                }

            private:
                using SourceSchema = Orm::Schema<TName, TItems ... >;
                using Items = typename SourceSchema::Items;
                using Indent = MIF_STATIC_STR("    ");

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
                    name = T::Value;
                    auto const sql = "CREATE SCHEMA " + name + ";\n";
                    name += ".";
                    return sql;
                }

                template <typename T, std::size_t I>
                static typename std::enable_if<I == 0, std::string>::type
                CreateItem(std::string const &schema)
                {
                    Common::Unused(schema);
                    return {};
                }

                template <typename T, std::size_t I>
                static typename std::enable_if<I != 0, std::string>::type
                CreateItem(std::string const &schema)
                {
                    auto sql = CreateItem<typename std::tuple_element<std::tuple_size<T>::value - I, Items>::type>(schema);
                    if (!sql.empty())
                        sql += "\n";
                    sql += CreateItem<T, I - 1>(schema);
                    return sql;
                }

                template <typename T>
                static typename std::enable_if<Orm::Detail::Traits::IsTable<T>(), std::string>::type
                CreateItem(std::string const &schema)
                {
                    using Type = typename T::Type;
                    using Meta = Reflection::Reflect<Type>;
                    std::string table = Meta::Name::Value;
                    auto sql = "CREATE TABLE " + schema + table + "\n";
                    sql += "(\n";
                    AppendField<T, 0, Meta::Fields::Count>(sql);
                    sql += ");\n";
                    return sql;
                }

                template <typename T, std::size_t I, std::size_t N>
                static typename std::enable_if<I != N, void>::type
                AppendField(std::string &sql)
                {
                    using Entity = typename T::Type;
                    using Field = typename Reflection::Reflect<Entity>::Fields::template Field<I>;
                    using Traits = Orm::Detail::Traits::MakeFieldTraitsList<Field, typename T::Traits>;

                    sql += Indent::Value;
                    sql += Detail::Utility::QuoteReserved(Field::Name::Value);
                    sql += " ";
                    sql += GetTypeName<typename Field::Type>();
                    if (N - I > 1)
                        sql += ",";
                    sql += "\n";

                    AppendField<T, I + 1, N>(sql);
                }

                template <typename T, std::size_t I, std::size_t N>
                static typename std::enable_if<I == N, void>::type
                AppendField(std::string &sql)
                {
                    Common::Unused(sql);
                }

                template <typename T>
                static typename std::enable_if<std::is_arithmetic<T>::value || std::is_same<T, std::string>::value, std::string>::type
                GetTypeName()
                {
                    return Detail::Type::Holder<T>::Name::Value;
                }

                template <typename T>
                static typename std::enable_if<std::is_enum<T>::value && Reflection::IsReflectable<T>(), std::string>::type
                GetTypeName()
                {
                    return Reflection::Reflect<T>::Name::Value;
                }

                template <typename T>
                static typename std::enable_if<std::is_enum<T>::value && !Reflection::IsReflectable<T>(), std::string>::type
                GetTypeName()
                {
                    return GetTypeName<typename std::underlying_type<T>::type>();
                }

                template <typename T>
                static typename std::enable_if<Serialization::Traits::IsSmartPointer<T>(), std::string>::type
                GetTypeName()
                {
                    return GetTypeName<typename T::element_type>();
                }

                template <typename T>
                static typename std::enable_if<Reflection::IsReflectable<T>() && !std::is_enum<T>::value, std::string>::type
                GetTypeName()
                {
                    // TODO: nested entity
                    return {};
                }

                template <typename T>
                static typename std::enable_if
                    <
                        Serialization::Traits::IsIterable<T>() &&
                        !std::is_same<T, std::string>::value &&
                        Reflection::IsReflectable<typename T::value_type>(),
                        std::string
                    >::type
                GetTypeName()
                {
                    // TODO: collection with reflectable struct (nested struct collection)
                    return {};
                }
            };

        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_POSTGRESQL_SCHEMA_H__
