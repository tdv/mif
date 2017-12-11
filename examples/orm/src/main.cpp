// STD
#include <memory>
#include <iostream>

// MIF
#include <mif/common/static_string.h>
#include <mif/reflection/reflect_type.h>
#include <mif/orm/postgresql/driver.h>
#include <mif/orm/storage.h>
#include <mif/application/iconfig.h>
#include <mif/application/id/config.h>
#include <mif/service/create.h>
#include <mif/db/iconnection_pool.h>
#include <mif/db/id/service.h>
#include <mif/db/transaction.h>

namespace Data
{

    enum class Position
    {
        Unknown,
        Developer,
        Manager
    };

    struct Additional
    {
        std::string descr;
    };

    struct Info
    {
        std::vector<Additional> meta;
        std::string address;
        std::shared_ptr<std::string> additional;
        boost::posix_time::ptime::date_type date;
        boost::posix_time::ptime timestamp;
        std::size_t counter;
    };

    struct Employee
    {
        std::size_t counter = 0;
        std::string name;
        std::string lastName;
        std::uint32_t age = 0;
        std::vector<std::string> email;
        Position position = Position::Unknown;
        double rate = 0.0;
        Info info;
    };

}   // namespace Data

namespace Data
{
    namespace Meta
    {

        MIF_REFLECT_BEGIN(Additional)
            MIF_REFLECT_FIELD(descr)
        MIF_REFLECT_END()

        MIF_REFLECT_BEGIN(Position)
            MIF_REFLECT_FIELD(Unknown)
            MIF_REFLECT_FIELD(Developer)
            MIF_REFLECT_FIELD(Manager)
        MIF_REFLECT_END()

        MIF_REFLECT_BEGIN(Info)
            MIF_REFLECT_FIELD(meta)
            MIF_REFLECT_FIELD(address)
            MIF_REFLECT_FIELD(additional)
            MIF_REFLECT_FIELD(date)
            MIF_REFLECT_FIELD(timestamp)
            MIF_REFLECT_FIELD(counter)
        MIF_REFLECT_END()

        MIF_REFLECT_BEGIN(Employee)
            MIF_REFLECT_FIELD(counter)
            MIF_REFLECT_FIELD(name)
            MIF_REFLECT_FIELD(lastName)
            MIF_REFLECT_FIELD(age)
            MIF_REFLECT_FIELD(email)
            MIF_REFLECT_FIELD(position)
            MIF_REFLECT_FIELD(rate)
            MIF_REFLECT_FIELD(info)
        MIF_REFLECT_END()

    }   // namespace Meta
}   // namespace Data


MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Position)
MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Additional)
MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Info)
MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Employee)


using Schema = Mif::Orm::Schema
    <
        /*Mif::Orm::DefailtSchemaName*/ MIF_STATIC_STR("user_schema"),
        typename Mif::Orm::Enum<Data::Position>::Create,
        typename Mif::Orm::Table<Data::Additional>::Create::OnlyInfo,
        typename Mif::Orm::Table<Data::Info>
                ::Field<MIF_FIELD_META(&Data::Info::address)>::NotNull::MultiPrimaryKey<MIF_FIELD_META(&Data::Info::counter)>
                ::Field<MIF_FIELD_META(&Data::Info::timestamp)>::NotNull::WithTimezone
                ::Field<MIF_FIELD_META(&Data::Info::counter)>::Counter::NotNull//::PrimaryKey
            ::Create::OnlyInfo,
        typename Mif::Orm::Table<Data::Employee>
                ::Field<MIF_FIELD_META(&Data::Employee::counter)>::Counter::NotNull::PrimaryKey
                ::Field<MIF_FIELD_META(&Data::Employee::lastName)>::NotNull
                ::Field<MIF_FIELD_META(&Data::Employee::name)>::NotNull
                ::Field<MIF_FIELD_META(&Data::Employee::email)>::NotNull//::Unique
            ::Create
    >;

int main()
{
    try
    {
        /*auto stream = std::make_shared<std::stringstream>();
        *stream << "<document>"
                    << "<storage>"
                        << "<host>localhost</host>"
                        << "<port>5432</port>"
                        << "<user>postgres</user>"
                        << "<password>postgres</password>"
                        << "<dbname>orm_test</dbname>"
                        << "<connectiontimeout>10</connectiontimeout>"
                    << "</storage>"
                << "</document>";

        auto config = Mif::Service::Create<Mif::Application::Id::Service::Config::Xml, Mif::Application::IConfig>(
                std::static_pointer_cast<std::istream>(stream));
        auto connections = Mif::Service::Create<Mif::Db::Id::Service::PostgresConnectionPool, Mif::Db::IConnectionPool>(config->GetConfig("storage"));

        auto conn = connections->GetConnection();
        Mif::Db::Transaction tr{conn};

        Mif::Orm::Storage<Schema, Mif::Orm::PostgreSql::Driver> storage{conn};

        //storage.Create();

        tr.Commit();*/

        using Sql = Mif::Orm::PostgreSql::Driver<Schema>;

        auto const schema = Sql::CreateSchema();

        std::cout << "BEGIN;\n\n" << schema << "\n\nROLLBACK;" << std::endl;
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
