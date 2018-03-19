// STD
#include <string>
#include <iostream>
#include <vector>
#include <map>

// MIF
#include <mif/reflection/reflect_type.h>
#include <mif/orm/structure.h>
#include <mif/orm/postgresql/driver.h>


// TODO: check created enums for table fields (Mif::Orm::Detail. Not in PG ns)
// TODO: check created tables for table fields (Mif::Orm::Detail. Not in PG ns)
// TODO: traits 'Reference' for reflectalbe iterable types

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
        std::uint32_t counter = 0;
        boost::posix_time::ptime timestamp;
        boost::posix_time::ptime::date_type date;
        std::string name;
        std::string country;
        std::string city;
        Code code = Code1;
    };

    struct Nested2
    {
        std::string nestedNestedSimpleField1;
        std::size_t nestedNestedSimpleField2 = 0;
    };

    struct Nested1
    {
        std::size_t nested1_id = 0;
        std::string nestedSimpleField1;
        std::size_t nestedSimpleField2 = 0;
        Nested2 complexField;
    };

    struct Address
    {
        std::size_t counter = 0;
        std::string country;
        std::string city;
        std::size_t code = 0;
        Status status = Status::Active;
        std::vector<std::string> items;
        Nested1 nested;
        Address1 linkedAddress;
        std::map<std::string, Nested1> mapData;
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
            MIF_REFLECT_FIELD(counter)
            MIF_REFLECT_FIELD(timestamp)
            MIF_REFLECT_FIELD(date)
            MIF_REFLECT_FIELD(name)
            MIF_REFLECT_FIELD(country)
            MIF_REFLECT_FIELD(city)
            MIF_REFLECT_FIELD(code)
        MIF_REFLECT_END()

        MIF_REFLECT_BEGIN(Nested2)
            MIF_REFLECT_FIELD(nestedNestedSimpleField1)
            MIF_REFLECT_FIELD(nestedNestedSimpleField2)
        MIF_REFLECT_END()

        MIF_REFLECT_BEGIN(Nested1)
            MIF_REFLECT_FIELD(nested1_id)
            MIF_REFLECT_FIELD(nestedSimpleField1)
            MIF_REFLECT_FIELD(nestedSimpleField2)
            MIF_REFLECT_FIELD(complexField)
        MIF_REFLECT_END()

        MIF_REFLECT_BEGIN(Address)
            MIF_REFLECT_FIELD(counter)
            MIF_REFLECT_FIELD(country)
            MIF_REFLECT_FIELD(city)
            MIF_REFLECT_FIELD(code)
            MIF_REFLECT_FIELD(status)
            MIF_REFLECT_FIELD(items)
            MIF_REFLECT_FIELD(nested)
            MIF_REFLECT_FIELD(linkedAddress)
            MIF_REFLECT_FIELD(mapData)
        MIF_REFLECT_END()

    }   // namespace Meta
}   // namespace Data


MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Status)
MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Address1)
MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Nested2)
MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Nested1)
MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Address)

int main()
{
    try
    {
        using Schema = typename Mif::Orm::Schema
            <
                MIF_STATIC_STR("MyNs"),
                typename Mif::Orm::Enum<Data::Status>::Create,
                typename Mif::Orm::Table<Data::Address1>
                        ::Field<MIF_FIELD_META(&Data::Address1::counter)>::NotNull::Counter::PrimaryKey
                        ::Field<MIF_FIELD_META(&Data::Address1::timestamp)>::NotNull::Unique::WithoutTimezone
                    ::Create,
                typename Mif::Orm::Table<Data::Address>
                        ::Field<MIF_FIELD_META(&Data::Address::country)>::NotNull::Unique
                        ::Field<MIF_FIELD_META(&Data::Address::code)>::NotNull
                        ::Field<MIF_FIELD_META(&Data::Address::counter)>::NotNull::Counter::PrimaryKey
                        ::Field<MIF_FIELD_META(&Data::Address::status)>::NotNull
                        ::Field<MIF_FIELD_META(&Data::Address::linkedAddress)>::Reference
                    ::Create
            >::Create;

        using Driver = Mif::Orm::PostgreSql::Driver<Schema>;

        auto const schemaSql = Driver::CreateSchema();

        //auto const sql = "BEGIN;\n\n" + schemaSql + "\nROLLBACK;\n";
        //std::cout << sql << std::endl;

        Data::Address1 address1{};
        auto const sql = Driver::Insert(address1);

        std::cout << "BEGIN;\n" << sql << "ROLLBACK;" << std::endl;
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
