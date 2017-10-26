//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_POSTGRESQL_DRIVER_H__
#define __MIF_ORM_POSTGRESQL_DRIVER_H__

// STD
#include <string>
#include <tuple>
#include <type_traits>

// MIF
#include "mif/common/detail/tuple_utility.h"
#include "mif/common/unused.h"
#include "mif/orm/detail/field_traits.h"
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
            class Driver;

            template <typename TName, typename ... TItems>
            class Driver<Orm::Schema<TName, TItems ... >> final
            {
            public:
                static std::string CreateSchema()
                {
                    auto sql = CreateSchema<SchemaName>();

                    if (!sql.empty())
                        sql += "\n";

                    sql += CreateItem<Items, std::tuple_size<Items>::value>();

                    return sql;
                }

            private:
                using SourceSchema = Orm::Schema<TName, TItems ... >;
                using SchemaName = typename std::conditional
                    <
                            std::is_same<typename SourceSchema::Name, DefailtSchemaName>::value,
                            MIF_STATIC_STR(""),
                            typename SourceSchema::Name
                    >::type;
                using Items = typename SourceSchema::Items;
                using Indent = MIF_STATIC_STR("    ");

                static std::string GetSchemaName()
                {
                    return SchemaName::Size > 1 ? std::string{SchemaName::Value} + "." : std::string{};
                }

                template <typename T>
                static typename std::enable_if<std::is_same<T, DefailtSchemaName>::value, std::string>::type
                CreateSchema()
                {
                    return {};
                }

                template <typename T>
                static typename std::enable_if<!std::is_same<T, DefailtSchemaName>::value, std::string>::type
                CreateSchema()
                {
                    auto const sql = "CREATE SCHEMA " + Detail::Utility::QuoteReserved(SchemaName::Value) + ";\n";
                    return sql;
                }

                template <typename T, std::size_t I>
                static typename std::enable_if<I == 0, std::string>::type
                CreateItem()
                {
                    return {};
                }

                template <typename T, std::size_t I>
                static typename std::enable_if<I != 0, std::string>::type
                CreateItem()
                {
                    auto sql = CreateItem<typename std::tuple_element<std::tuple_size<T>::value - I, Items>::type>();
                    if (!sql.empty())
                        sql += "\n";
                    sql += CreateItem<T, I - 1>();
                    return sql;
                }

                template <typename T>
                static typename std::enable_if<Orm::Detail::Traits::IsTable<T>(), std::string>::type
                CreateItem()
                {
                    // TODO: append checking unique tables
                    // TODO: append checking unique primary key (only one primary key must be in table)
                    using Type = typename T::Type;
                    using Meta = Reflection::Reflect<Type>;
                    std::string name = Meta::Name::Value;
                    auto sql = "CREATE TABLE " + Detail::Utility::QuoteReserved(GetSchemaName() + name) + "\n";
                    sql += "(\n";
                    AppendField<T, 0, Meta::Fields::Count>(sql);
                    sql += ");\n";
                    return sql;
                }

                template <typename T>
                static typename std::enable_if<Orm::Detail::Traits::IsEnum<T>(), std::string>::type
                CreateItem()
                {
                    // TODO: append checking unique enums
                    using Type = typename T::Type;
                    using Meta = Reflection::Reflect<Type>;
                    std::string name = Detail::Utility::QuoteReserved(GetSchemaName() + Meta::Name::Value);
                    auto sql = "CREATE TYPE " + name + " AS ENUM\n";
                    sql += "(\n";
                    AppendEnumValue<Type, Meta::Fields::Count>(sql);
                    sql += ");\n";
                    return sql;
                }

                template <typename T, std::size_t I>
                static typename std::enable_if<I != 0, void>::type
                AppendEnumValue(std::string &sql)
                {
                    AppendEnumValue<T, I - 1>(sql);

                    using Fields = typename Reflection::Reflect<T>::Fields;
                    using Field = typename Fields::template Field<I - 1>;
                    sql += Indent::Value;
                    sql += Detail::Utility::QuoteValue(Field::Name::Value);
                    if (Fields::Count - I > 1)
                        sql += ",";
                    sql += "\n";
                }

                template <typename T, std::size_t I>
                static typename std::enable_if<I == 0, void>::type
                AppendEnumValue(std::string &sql)
                {
                    Common::Unused(sql);
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
                    sql += ExpandFieldInfo<typename Field::Type, Traits>();
                    sql += NullableField<Traits>();
                    sql += UniqueField<Traits>();
                    sql += PrimaryKeyField<Traits>();
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

                template <typename T, typename TTraits>
                static typename std::enable_if
                    <
                        std::is_arithmetic<T>::value || std::is_same<T, std::string>::value ||
                        std::is_same<T, boost::posix_time::ptime::date_type>::value ||
                        std::is_same<T, boost::posix_time::ptime>::value,
                        std::string
                    >::type
                ExpandFieldInfo()
                {
                    constexpr auto counter = Common::Detail::TupleContains<Orm::Detail::FieldTraits::Counter, TTraits>::value;
                    constexpr auto isUInt32 = std::is_same<T, std::uint32_t>::value;
                    constexpr auto isUInt64 = std::is_same<T, std::uint64_t>::value;
                    static_assert(!counter || (isUInt32 || isUInt64), "[Mif::Orm::PostgreSql::Driver::ExpandFieldInfo] "
                            "The counter field must be uint32 or uint64.");

                    constexpr auto isTimestamp = std::is_same<T, boost::posix_time::ptime>::value;
                    constexpr auto timezone = Common::Detail::TupleContains<Orm::Detail::FieldTraits::WithTimezone, TTraits>::value;

                    static_assert(!timezone || isTimestamp, "[Mif::Orm::PostgreSql::Driver::ExpandFieldInfo] "
                            "The 'WITH TIMEZONE' attribute should only be used for date or timestamp types.");

                    std::string type = std::conditional
                            <
                                counter,
                                typename std::conditional<isUInt32, Detail::Type::Serial, Detail::Type::BigSerial>::type,
                                typename Detail::Type::Holder<T>::Name
                            >::type::Value;

                    if (timezone)
                        type += (timezone) ? " WITH TIME ZONE" : " WITHOUT TIME ZONE";

                    return type;
                }

                template <typename T, typename TTraits>
                static typename std::enable_if<std::is_enum<T>::value && Reflection::IsReflectable<T>(), std::string>::type
                ExpandFieldInfo()
                {
                    // TODO: not a counter, no TZ
                    return Detail::Utility::QuoteReserved(GetSchemaName() + Reflection::Reflect<T>::Name::Value);
                }

                template <typename T, typename TTraits>
                static typename std::enable_if<std::is_enum<T>::value && !Reflection::IsReflectable<T>(), std::string>::type
                ExpandFieldInfo()
                {
                    // TODO: not a counter, no TZ
                    return ExpandFieldInfo<typename std::underlying_type<T>::type>();
                }

                template <typename T, typename TTraits>
                static typename std::enable_if<Serialization::Traits::IsSmartPointer<T>(), std::string>::type
                ExpandFieldInfo()
                {
                    constexpr auto notNull = Common::Detail::TupleContains<Orm::Detail::FieldTraits::NotNull, TTraits>::value;
                    static_assert(!notNull, "[Mif::Orm::PostgreSql::Driver::ExpandFieldInfo] "
                            "The pointer field must not be NOT NULL.");
                    return ExpandFieldInfo<typename T::element_type, TTraits>();
                }

                template <typename T, typename TTraits>
                static typename std::enable_if<Reflection::IsReflectable<T>() && !std::is_enum<T>::value, std::string>::type
                ExpandFieldInfo()
                {
                    // TODO: nested entity
                    return {};
                }

                template <typename T, typename TTraits>
                static typename std::enable_if
                    <
                        Serialization::Traits::IsIterable<T>() &&
                        !std::is_same<T, std::string>::value &&
                        Reflection::IsReflectable<typename T::value_type>(),
                        std::string
                    >::type
                ExpandFieldInfo()
                {
                    // TODO: collection with reflectable struct (nested struct collection)
                    return {};
                }

                template <typename TTraits>
                static std::string NullableField()
                {
                    constexpr auto null = Common::Detail::TupleContains<Orm::Detail::FieldTraits::Nullable, TTraits>::value;
                    constexpr auto notNull = Common::Detail::TupleContains<Orm::Detail::FieldTraits::NotNull, TTraits>::value;
                    static_assert(!null || !notNull, "[Mif::Orm::PostgreSql::Schema;:NullableField] "
                            "Conflicting NULL/NOT NULL declarations for column.");
                    return null ? " NULL" : notNull ? " NOT NULL" : "";
                }

                template <typename TTraits>
                static std::string UniqueField()
                {
                    constexpr auto unique = Common::Detail::TupleContains<Orm::Detail::FieldTraits::Unique, TTraits>::value;
                    return unique ? " UNIQUE" : "";
                }

                template <typename TTraits>
                static std::string PrimaryKeyField()
                {
                    // TODO: support multi primary key
                    constexpr auto pk = Common::Detail::TupleContains<Orm::Detail::FieldTraits::PrimaryKey<>, TTraits>::value;
                    return pk ? " PRIMARY KEY" : "";
                }
            };

        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_POSTGRESQL_DRIVER_H__
