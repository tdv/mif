//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_POSTGRESQL_DRIVER_H__
#define __MIF_ORM_POSTGRESQL_DRIVER_H__

// MIF
#include "mif/common/config.h"
#ifdef MIF_WITH_POSTGRESQL

// STD
#include <algorithm>
#include <iterator>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>

// BOOST
#include <boost/algorithm/string.hpp>

// MIF
#include "mif/common/detail/tuple_utility.h"
#include "mif/common/unused.h"
#include "mif/orm/detail/utility.h"
#include "mif/orm/postgresql/detail/type_holder.h"
#include "mif/orm/postgresql/detail/utility.h"
#include "mif/orm/schema.h"
#include "mif/reflection/reflection.h"
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

                    sql += CreateItem<Items, std::tuple_size<Items>::value, std::tuple<>>();

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

                template <typename T, std::size_t I, typename TRes>
                static typename std::enable_if<I == 0, std::string>::type
                CreateItem()
                {
                    return {};
                }

                template <typename T, std::size_t I, typename TRes>
                static typename std::enable_if<I != 0, std::string>::type
                CreateItem()
                {
                    using Item = typename std::tuple_element<std::tuple_size<T>::value - I, Items>::type;

                    static_assert(!Common::Detail::TupleContains<Item, TRes>::value,
                            "[Mif::Orm::PostgreSql::Driver::CreateItem] In the schema all elements must be unique."
                        );

                    auto sql = CreateItem<Item>();
                    if (!sql.empty())
                        sql += "\n";
                    sql += CreateItem<T, I - 1, typename Common::Detail::TupleCat<std::tuple<Item>, TRes>::Tuple>();

                    return sql;
                }

                template <typename T>
                static typename std::enable_if<Orm::Detail::Traits::IsTable<T>(), std::string>::type
                CreateItem()
                {
                    using Type = typename T::Type;
                    using Meta = Reflection::Reflect<Type>;
                    std::string name = Meta::Name::Value;
                    auto sql = "CREATE TABLE " + Detail::Utility::QuoteReserved(GetSchemaName() + name) + "\n";
                    sql += "(\n";
                    AppendField<T, Meta::Fields::Count>(sql);

                    auto const pkMulti = MultiPrimaryKey<typename Meta::Fields, typename T::Traits>();
                    if (!pkMulti.empty())
                    {
                        auto pos = sql.find_last_not_of('\n');
                        if (pos != std::string::npos)
                            sql.erase(pos + 1);
                        sql += ",\n";
                        sql += pkMulti;
                        sql += "\n";
                    }

                    sql += ");\n";
                    return sql;
                }

                template <typename T>
                static typename std::enable_if<Orm::Detail::Traits::IsEnum<T>(), std::string>::type
                CreateItem()
                {
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
                    if (Fields::Count - I > 0)
                        sql += ",";
                    sql += "\n";
                }

                template <typename T, std::size_t I>
                static typename std::enable_if<I == 0, void>::type
                AppendEnumValue(std::string &sql)
                {
                    Common::Unused(sql);
                }

                template <typename T, std::size_t I>
                static typename std::enable_if<I != 0, void>::type
                AppendField(std::string &sql)
                {
                    AppendField<T, I - 1>(sql);

                    using Entity = typename T::Type;
                    using Fields = typename Reflection::Reflect<Entity>::Fields;
                    using Field = typename Fields::template Field<I - 1>;
                    using Name = typename Field::Name;
                    using FieldType = typename Field::Type;
                    using Traits = Orm::Detail::Traits::MakeFieldTraitsList<Field, typename T::Traits>;

                    CheckFieldTraits<FieldType, Traits>();

                    sql += Indent::Value;

                    std::string linkedTable;
                    sql += ExpandFieldInfo<Name, FieldType, Traits>(linkedTable);

                    sql += WithTimezoneField<FieldType, Traits>();
                    sql += NullableField<Traits>();
                    sql += UniqueField<Traits>();
                    sql += PrimaryKeyField<Traits>();

                    if (Fields::Count - I > 0)
                        sql +=  "," ;

                    sql += "\n";
                }

                template <typename T, std::size_t I>
                static typename std::enable_if<I == 0, void>::type
                AppendField(std::string &sql)
                {
                    Common::Unused(sql);
                }

                template <typename TFName, typename T, typename TTraits>
                static typename std::enable_if
                    <
                        std::is_arithmetic<T>::value || std::is_same<T, std::string>::value ||
                        std::is_same<T, boost::posix_time::ptime::date_type>::value ||
                        std::is_same<T, boost::posix_time::ptime>::value,
                        std::string
                    >::type
                ExpandFieldInfo(std::string &linkedTable)
                {
                    Common::Unused(linkedTable);

                    constexpr auto counter = Common::Detail::TupleContains<Orm::Detail::FieldTraits::Counter, TTraits>::value;
                    constexpr auto isUInt32 = std::is_same<T, std::uint32_t>::value;
                    constexpr auto isUInt64 = std::is_same<T, std::uint64_t>::value;

                    auto const name = Detail::Utility::QuoteReserved(TFName::Value) + " ";

                    return name + std::conditional
                            <
                                counter,
                                typename std::conditional
                                    <
                                        isUInt32,
                                        Detail::Type::Serial,
                                        typename std::conditional
                                            <
                                                isUInt64,
                                                Detail::Type::BigSerial,
                                                typename Detail::Type::Holder<T>::Name
                                            >::type
                                    >::type,
                                typename Detail::Type::Holder<T>::Name
                            >::type::Value;
                }

                template <typename TFName, typename T, typename TTraits>
                static typename std::enable_if<std::is_enum<T>::value && Reflection::IsReflectable<T>(), std::string>::type
                ExpandFieldInfo(std::string &linkedTable)
                {
                    Common::Unused(linkedTable);

                    auto const name = Detail::Utility::QuoteReserved(TFName::Value) + " ";
                    return name + Detail::Utility::QuoteReserved(GetSchemaName() + Reflection::Reflect<T>::Name::Value);
                }

                template <typename TFName, typename T, typename TTraits>
                static typename std::enable_if<std::is_enum<T>::value && !Reflection::IsReflectable<T>(), std::string>::type
                ExpandFieldInfo(std::string &linkedTable)
                {
                    return ExpandFieldInfo<TFName, typename std::underlying_type<T>::type, TTraits>(linkedTable);
                }

                template <typename TFName, typename T, typename TTraits>
                static typename std::enable_if<Serialization::Traits::IsSmartPointer<T>(), std::string>::type
                ExpandFieldInfo(std::string &linkedTable)
                {
                    return ExpandFieldInfo<TFName, typename T::element_type, TTraits>(linkedTable);
                }

                template <typename TFName, typename T, typename TTraits>
                static typename std::enable_if<Reflection::IsReflectable<T>() && !std::is_enum<T>::value, std::string>::type
                ExpandFieldInfo(std::string &linkedTable)
                {
                    // TODO: add implementation for reflectable type

                    using Item = Orm::Detail::Utility::SelectTableItem<T, Items>;
                    using Entity = typename Item::Type;
                    using Traits = typename Item::Traits;
                    using Meta = typename Reflection::Reflect<Entity>;
                    using PrimaryKeyFields = Orm::Detail::Traits::PrimaryKeyFields<typename Meta::Fields, Traits>;
                    constexpr auto hasPrimaryKey = std::tuple_size<PrimaryKeyFields>::value;
                    static_assert(hasPrimaryKey, "[Mif::Orm::PostgreSql::Driver::ExpandFieldInfo] "
                            "Can not create a relationship for tables. There is no primary key in the owner table."
                        );

                    linkedTable = "CREATE TABLE " + Detail::Utility::QuoteReserved(GetSchemaName() + TFName::Value) + "\n";
                    linkedTable += "(\n";

                    linkedTable += ");\n";

                    return {};
                }

                template <typename T, typename TTraits>
                static typename std::enable_if
                    <
                        Serialization::Traits::IsIterable<T>() &&
                        Reflection::IsReflectable<typename T::value_type>(),
                        std::string
                    >::type
                ExpandFieldInfo(std::string &linkedTable)
                {
                    // TODO: add implementation for collections of reflectable types
                    return {};
                }

                template <typename TFName, typename T, typename TTraits>
                static typename std::enable_if
                    <
                        Serialization::Traits::IsIterable<T>() &&
                        !Reflection::IsReflectable<typename T::value_type>(),
                        std::string
                    >::type
                ExpandFieldInfo(std::string &linkedTable)
                {
                    // TODO: add implementation for collections of not reflectable types
                    return {};
                }

                template <typename TTraits>
                static std::string NullableField()
                {
                    constexpr auto null = Common::Detail::TupleContains<Orm::Detail::FieldTraits::Nullable, TTraits>::value;
                    constexpr auto notNull = Common::Detail::TupleContains<Orm::Detail::FieldTraits::NotNull, TTraits>::value;

                    return null ? " NULL" : notNull ? " NOT NULL" : "";
                }

                template <typename TTraits>
                static std::string UniqueField()
                {
                    constexpr auto unique = Common::Detail::TupleContains<Orm::Detail::FieldTraits::Unique, TTraits>::value;
                    return unique ? " UNIQUE" : "";
                }

                template <typename TField, typename TTraits>
                static std::string WithTimezoneField()
                {
                    constexpr auto timestamp = std::is_same<TField, boost::posix_time::ptime>::value;
                    if (!timestamp)
                        return {};
                    constexpr auto timezone = Common::Detail::TupleContains<Orm::Detail::FieldTraits::WithTimezone, TTraits>::value;
                    return (timezone) ? " WITH TIME ZONE" : " WITHOUT TIME ZONE";
                }

                template <typename TTraits>
                static std::string PrimaryKeyField()
                {
                    using PrimaryKey = Orm::Detail::Traits::PrimaryKeyTrait<TTraits>;
                    if (!PrimaryKey::value)
                        return {};
                    return !std::tuple_size<typename PrimaryKey::CoFields>::value ? "PRIMARY KEY" : "";
                }

                template <typename TFields, typename TTraits>
                static std::string MultiPrimaryKey()
                {
                    using PkFields = Orm::Detail::Traits::PrimaryKeyFields<TFields, TTraits>;
                    if (std::tuple_size<PkFields>::value < 2)
                        return {};
                    std::string sql = Indent::Value;
                    sql += " PRIMARY KEY (";
                    auto const names = Orm::Detail::Utility::CreateNameSet<PkFields>();
                    std::set<std::string> quotedNames;
                    std::transform(std::begin(names), std::end(names),
                            std::inserter(quotedNames, std::begin(quotedNames)),
                            [] (std::string const &str) { return Detail::Utility::QuoteReserved(str); }
                        );
                    sql += boost::algorithm::join(quotedNames, ", ");
                    sql += ")";
                    return sql;
                }

                template <typename TField, typename TTraits>
                static void CheckFieldTraits()
                {
                    // The counter field must be a unsigned integer of 32 or 64 bits
                    {
                        constexpr auto counter = Common::Detail::TupleContains<Orm::Detail::FieldTraits::Counter, TTraits>::value;
                        constexpr auto isUInt32 = std::is_same<TField, std::uint32_t>::value;
                        constexpr auto isUInt64 = std::is_same<TField, std::uint64_t>::value;

                        static_assert(!counter || (counter && (isUInt32 || isUInt64)), "[Mif::Orm::PostgreSql::Driver::CheckFieldTraits] "
                                "The counter field must be uint32 or uint64.");
                    }

                    // The TIMEZONE trait must be set only for DATE or TIMESTAMP field type
                    {
                        constexpr auto timestamp = std::is_same<TField, boost::posix_time::ptime>::value;
                        constexpr auto timezone = Common::Detail::TupleContains<Orm::Detail::FieldTraits::WithTimezone, TTraits>::value;

                        static_assert(!timezone || timestamp, "[Mif::Orm::PostgreSql::Driver::CheckFieldTraits] "
                                "The 'WITH TIMEZONE' attribute should only be used for date or timestamp types.");
                    }

                    // The smart pointer field must not be NOT NULL
                    {
                        constexpr auto smartPointer = Serialization::Traits::IsSmartPointer<TField>();
                        constexpr auto notNull = Common::Detail::TupleContains<Orm::Detail::FieldTraits::NotNull, TTraits>::value;

                        static_assert(!smartPointer || !notNull, "[Mif::Orm::PostgreSql::Driver::CheckFieldTraits] "
                                "The pointer field must not be NOT NULL.");
                    }

                    // NULL and NOT NULL must not be together set
                    {
                        constexpr auto null = Common::Detail::TupleContains<Orm::Detail::FieldTraits::Nullable, TTraits>::value;
                        constexpr auto notNull = Common::Detail::TupleContains<Orm::Detail::FieldTraits::NotNull, TTraits>::value;

                        static_assert(!null || !notNull, "[Mif::Orm::PostgreSql::Schema;:CheckFieldTraits] "
                                "Conflicting NULL/NOT NULL declarations for column.");
                    }

                    // TODO: all of the primary key fields must be a simple type
                }
            };

        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Mif

#endif  // !MIF_WITH_POSTGRESQL
#endif  // !__MIF_ORM_POSTGRESQL_DRIVER_H__
