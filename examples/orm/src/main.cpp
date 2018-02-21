// STD
#include <list>
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

    struct Address1
    {
        std::string country;
        std::string city;
        std::size_t code = 0;
    };

    struct Address
    {
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
            MIF_REFLECT_FIELD(country)
            MIF_REFLECT_FIELD(city)
            MIF_REFLECT_FIELD(code)
        MIF_REFLECT_END()

        MIF_REFLECT_BEGIN(Address)
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
        namespace PostgreSql
        {
            namespace Detail
            {
                namespace Utility
                {

                    inline std::string Quote(std::string const &str)
                    {
                        std::string tmp = str;
                        boost::replace_all(tmp, "'", "''");
                        return "'" + tmp + "'";
                    }

                    inline std::string QuoteReserved(std::string const &str)
                    {
                        // TODO: do quote for reserved name for PostgreSql

                        return str;
                    }

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

                using StringList = std::list<std::string>;

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

                template <typename TSchemaName, typename TEntity, typename ... TEntities>
                class Entity<TSchemaName, Orm::Detail::Entity<Orm::Table<TEntity>>, TEntities ... > final
                {
                public:
                    static void Create(StringList &items)
                    {
                        auto sql = "CREATE TABLE " + EntityName<TSchemaName, typename Meta::Name>::Create() + "\n";
                        sql += "(\n";

                        // TODO:

                        sql += ");\n";
                        items.emplace_back(sql);
                        Entity<TSchemaName, TEntities ... >::Create(items);
                    }

                private:
                    using Meta = Reflection::Reflect<TEntity>;
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
                        ::Field<MIF_FIELD_META(&Data::Address::code)>::NotNull::PrimaryKey
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
