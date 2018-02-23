// STD
#include <list>
#include <limits>
#include <string>
#include <iostream>
#include <tuple>
#include <type_traits>

// MIF
#include <mif/common/unused.h>
#include <mif/common/static_string.h>
#include <mif/reflection/reflect_type.h>
#include <mif/reflection/reflection.h>
#include <mif/orm/structure.h>
#include <mif/orm/postgresql/detail/utility.h>

// BOOST
#include <boost/algorithm/string.hpp>

namespace Data
{

    enum class Status
    {
        Unknown,
        Active,
        Locked
    };

    enum Code
    {
        Code1, Code2
    };

    struct Address1
    {
        std::string name;
        std::string country;
        std::string city;
        Code code = Code1;
    };

    struct Address
    {
        std::size_t counter = 0;
        std::string country;
        std::string city;
        std::size_t code = 0;
    };

}   // namespace Data

namespace Data
{
    namespace Meta
    {

        MIF_REFLECT_BEGIN(Status)
            MIF_REFLECT_FIELD(Unknown)
            MIF_REFLECT_FIELD(Active)
            MIF_REFLECT_FIELD(Locked)
        MIF_REFLECT_END()

        MIF_REFLECT_BEGIN(Address1)
            MIF_REFLECT_FIELD(name)
            MIF_REFLECT_FIELD(country)
            MIF_REFLECT_FIELD(city)
            MIF_REFLECT_FIELD(code)
        MIF_REFLECT_END()

        MIF_REFLECT_BEGIN(Address)
            MIF_REFLECT_FIELD(counter)
            MIF_REFLECT_FIELD(country)
            MIF_REFLECT_FIELD(city)
            MIF_REFLECT_FIELD(code)
        MIF_REFLECT_END()

    }   // namespace Meta
}   // namespace Data


MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Status)
MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Address1)
MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Address)

namespace Mif
{
    namespace Orm
    {
        namespace Detail
        {
            namespace Utility
            {

                template <typename TTable, typename TFieldMeta>
                struct FieldTraits
                {
                    using Traits = std::tuple<>;
                };

                template <typename TFieldMeta, typename TEntity, typename ... TTraits>
                struct FieldTraits<Orm::Detail::Entity<Table<TEntity, TTraits ... >>, TFieldMeta>
                {
                    using Traits = std::tuple<>;
                };

            }   // namespace Utility
        }   // namespace Detail
    }   // namespace Orm
}   // namespace Mif

namespace Mif
{
    namespace Orm
    {
        namespace PostgreSql
        {
            namespace Detail
            {
                using StringList = std::list<std::string>;

                namespace Utility
                {
                    namespace Type
                    {
                        namespace Simple
                        {

                            template <typename>
                            struct Holder;

    #define MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(type_, name_) \
        template <> \
        struct Holder<type_> \
        { \
            using Name = MIF_STATIC_STR(name_); \
            using Type = type_; \
        };
                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(bool, "INTEGERU")
                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(char, "INTEGER")
                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(unsigned char, "INTEGER")
                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(short, "INTEGER")
                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(unsigned short, "INTEGER")
                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(int, "INTEGER")
                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(unsigned int, "INTEGER")

                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(long, "BIGINT")
                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(unsigned long, "BIGINT")
                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(long long, "BIGINT")
                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(unsigned long long, "BIGINT")

                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(float, "REAL")
                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(double, "DOUBLE PRECISION")

                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(std::string, "TEXT")

                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(boost::posix_time::ptime::date_type, "DATE")
                            MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(boost::posix_time::ptime, "TIMESTAMP")

                            using Serial = MIF_STATIC_STR("SERIAL");
                            using BigSerial = MIF_STATIC_STR("BIGSERIAL");

    #undef MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL

                            struct TypeName final
                            {
                                template <typename T>
                                static typename std::enable_if<!std::is_enum<T>::value, std::string>::type Get()
                                {
                                    return Holder<T>::Name::Value;
                                }

                                template <typename T>
                                static typename std::enable_if<std::is_enum<T>::value, std::string>::type Get()
                                {
                                    return Holder<typename std::underlying_type<T>::type>::Name::Value;
                                }
                            };

                        }   // namespace Simple
                    }   // namespace Type

                    namespace FieldTraits
                    {
                        namespace Trait
                        {

                            // TODO: traits to strings
                            template <typename>
                            struct ToString final
                            {
                                using Name = MIF_STATIC_STR("TRAIT");
                            };

                        }   // namespace Trait


                        template <typename TTrait, typename TTraits, bool = Common::Detail::TupleContains<TTrait, TTraits>::value>
                        struct AppendTrait final
                        {
                            static void Append(StringList &traits)
                            {
                                traits.emplace_back(Trait::ToString<TTrait>::Name::Value);
                            }
                        };

                        template <typename TTrait, typename TTraits>
                        struct AppendTrait<TTrait, TTraits, false> final
                        {
                            static void Append(StringList &traits)
                            {
                                Common::Unused(traits);
                            }
                        };

                        template <typename TTraits, typename TAllTraits>
                        struct TupleToStringList;

                        template <typename TTraits, typename TCur, typename ... TTail>
                        struct TupleToStringList<TTraits, std::tuple<TCur, TTail ... >> final
                        {
                            static void Get(StringList &traits)
                            {
                                AppendTrait<TCur, TTraits>::Append(traits);
                                TupleToStringList<TTraits, std::tuple<TTail ... >>::Get(traits);
                            }
                        };

                        template <typename TTraits>
                        struct TupleToStringList<TTraits, std::tuple<>> final
                        {
                            static void Get(StringList &traits)
                            {
                                Common::Unused(traits);
                            }
                        };

                        template <typename TTraits>
                        struct ToStringList final
                        {
                            static void Get(StringList &traits)
                            {
                                TupleToStringList<TTraits, Orm::Detail::FieldTraits::AllTypeTraits>::Get(traits);
                            }
                        };

                    }   // namespace FieldTraits
                }   // namespace Utility
            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Mif

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
                        return Utility::QuoteReserved(name);
                    }
                };

                template <typename ... T>
                class Entity final
                {
                public:
                    static void Create(StringList &items)
                    {
                        Common::Unused(items);
                    }

                private:
                    template <char const ... Ch>
                    static constexpr std::true_type IsName(Common::StaticString<Ch ... > const *);
                    static constexpr std::false_type IsName(...);

                    static_assert(sizeof ... (T) == 1 &&
                            decltype(IsName(static_cast<typename std::tuple_element<0, std::tuple<T ... >>::type const *>(nullptr)))::value,
                            "[Mif::Orm::PostgreSql::Detail::Entity] Unknown object to generate sql."
                        );
                };

                template <typename TSchemaName, typename TEntity, typename ... TEntities>
                class Entity<TSchemaName, Orm::Detail::Entity<Orm::Enum<TEntity>>, TEntities ... > final
                {
                public:
                    static void Create(StringList &items)
                    {
                        auto sql = "CREATE TYPE " + EntityName<TSchemaName, typename Meta::Name>::Create() + " AS ENUM\n";
                        sql += "(\n";
                        StringList enumItems;
                        CreateItems<Meta::Fields::Count>(enumItems);
                        sql += boost::algorithm::join(enumItems, ",\n");
                        if (!enumItems.empty())
                            sql += "\n";
                        sql += ");\n";
                        items.emplace_back(sql);
                        Entity<TSchemaName, TEntities ... >::Create(items);
                    }

                private:
                    using Meta = Reflection::Reflect<TEntity>;

                    template <std::size_t I>
                    static typename std::enable_if<I == 0, void>::type
                    CreateItems(StringList &items)
                    {
                        Common::Unused(items);
                    }

                    template <std::size_t I>
                    static typename std::enable_if<I != 0, void>::type
                    CreateItems(StringList &items)
                    {
                        using Field = typename Meta::Fields::template Field<Meta::Fields::Count - I>;
                        items.emplace_back(Indent::Value + Utility::Quote(Field::Name::Value));
                        CreateItems<I - 1>(items);
                    }
                };

                template <typename TSchemaName, typename TEntity, typename ... TTableTraits, typename ... TEntities>
                class Entity<TSchemaName, Orm::Detail::Entity<Orm::Table<TEntity, TTableTraits ... >>, TEntities ... > final
                {
                public:
                    static void Create(StringList &items)
                    {
                        auto sql = "CREATE TABLE " + EntityName<TSchemaName, typename Meta::Name>::Create() + "\n";
                        sql += "(\n";
                        StringList tableItems;
                        CreateItem<Meta::Fields::Count>(tableItems);
                        sql += boost::algorithm::join(tableItems, ",\n");
                        if (!tableItems.empty())
                            sql += "\n";
                        sql += ");\n";
                        items.emplace_back(sql);
                        Entity<TSchemaName, TEntities ... >::Create(items);
                    }

                private:
                    using Table = Orm::Detail::Entity<Orm::Table<TEntity, TTableTraits ... >>;
                    using Meta = Reflection::Reflect<TEntity>;

                    // Simple types: all int, float pointers, text, datetimes, not reflectable enums
                    template <typename TField>
                    static typename std::enable_if
                        <
                            Serialization::Traits::IsSimple<typename TField::Type>() ||
                                std::is_same<typename TField::Type, boost::posix_time::ptime>::value ||
                                std::is_same<typename TField::Type, boost::posix_time::ptime::date_type>::value ||
                                (std::is_enum<typename TField::Type>::value && !Reflection::IsReflectable<typename TField::Type>()),
                            std::string
                        >::type
                    CreateItem()
                    {
                        std::string sql = Detail::Indent::Value;
                        sql += Utility::QuoteReserved(TField::Name::Value);
                        sql += " ";
                        sql += Utility::Type::Simple::TypeName::Get<typename TField::Type>();
                        StringList traits;
                        using Traits = typename Orm::Detail::Utility::FieldTraits<Table, TField>::Traits;
                        Detail::Utility::FieldTraits::ToStringList<Traits>::Get(traits);
                        if (!traits.empty())
                        {
                            sql += " ";
                            sql += boost::algorithm::join(traits, " ");
                        }
                        return sql;
                    }

                    template <std::size_t I>
                    static typename std::enable_if<I != 0, void>::type
                    CreateItem(StringList &items)
                    {
                        using Field = typename Meta::Fields::template Field<Meta::Fields::Count - I>;
                        auto sql = CreateItem<Field>();
                        items.emplace_back(std::move(sql));
                        CreateItem<I - 1>(items);
                    }

                    template <std::size_t I>
                    static typename std::enable_if<I == 0, void>::type
                    CreateItem(StringList &items)
                    {
                        Common::Unused(items);
                    }
                };

                template <typename TSchemaName, typename TEntity, typename ... TEntities>
                class Entity<Orm::Detail::Entity<Orm::Schema<TSchemaName, TEntity, TEntities ... >>> final
                {
                public:
                    static void Create(StringList &items)
                    {
                        CreateSchema<TSchemaName>(items);
                        Entity<TSchemaName, TEntity, TEntities ... >::Create(items);
                    }

                private:
                    using SchemaName = typename std::conditional
                        <
                            std::is_same<TSchemaName, Orm::DefaultSchemaName>::value,
                            DefaultSchemaName,
                            TSchemaName
                        >::type;

                    template <typename T>
                    static typename std::enable_if<std::is_same<T, Orm::DefaultSchemaName>::value, void>::type
                    CreateSchema(StringList &items)
                    {
                        Common::Unused(items);
                    }

                    template <typename T>
                    static typename std::enable_if<!std::is_same<T, Orm::DefaultSchemaName>::value, void>::type
                    CreateSchema(StringList &items)
                    {
                        items.emplace_back("CREATE SCHEMA " + Utility::QuoteReserved(SchemaName::Value) + ";\n");
                    }
                };

            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Mif

int main()
{
    try
    {
        using Schema = typename Mif::Orm::Schema
            <
                MIF_STATIC_STR("my_ns"),
                typename Mif::Orm::Enum<Data::Status>::Create,
                typename Mif::Orm::Table<Data::Address1>::Create,
                typename Mif::Orm::Table<Data::Address>
                        ::Field<MIF_FIELD_META(&Data::Address::country)>::NotNull::Unique
                        ::Field<MIF_FIELD_META(&Data::Address::code)>::NotNull
                        ::Field<MIF_FIELD_META(&Data::Address::counter)>::NotNull::Counter::PrimaryKey
                    ::Create
            >::Create;

        Mif::Orm::PostgreSql::Detail::StringList items;
        Mif::Orm::PostgreSql::Detail::Entity<Schema>::Create(items);
        auto const sql = boost::algorithm::join(items, "\n");
        std::cout << sql << std::endl;
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
