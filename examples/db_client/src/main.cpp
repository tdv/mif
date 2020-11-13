//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

// STD
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>

// BOOST
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/scope_exit.hpp>

// MIF
#include <mif/application/application.h>
#include <mif/common/uuid_generator.h>
#include <mif/common/log.h>
#include <mif/db/transaction.h>
#include <mif/db/id/service.h>
#include <mif/service/create.h>

class Application
    : public Mif::Application::Application
{
public:
    Application(int argc, char const **argv)
        : Mif::Application::Application{argc, argv}
    {
        boost::program_options::options_description commonOptions{"Common database options"};
        commonOptions.add_options()
                ("type", boost::program_options::value<std::string>(&m_type)->default_value("sqlite"),
                        "Database type (postgres or sqlite)")
                ("db-prefix", boost::program_options::value<std::string>(&m_dbPrefix)->default_value("mif_db_test"),
                    "Database name prefix")
                ("clean", boost::program_options::value<bool>(&m_cleanResult)->default_value(true),
                    "Remove all generated data")
                ;
        AddCustomOptions(commonOptions);

        boost::program_options::options_description pgOptions{"PostgreSQL options"};
        pgOptions.add_options()
                ("pg-host", boost::program_options::value<std::string>(&m_pgHost)->default_value("localhost"),
                        "PostgreSQL database host")
                ("pg-port", boost::program_options::value<std::uint16_t>(&m_pgPort)->default_value(5432),
                        "PostgreSQL database port")
                ("pg-user", boost::program_options::value<std::string>(&m_pgUser)->default_value("postgres"),
                        "PostgreSQL database user")
                ("pg-pwd", boost::program_options::value<std::string>(&m_pgPassword)->default_value(""),
                        "PostgreSQL database user password")
                ("pg-timeout", boost::program_options::value<std::uint32_t>(&m_pgConnectionTimeout)->default_value(10),
                        "PostgreSQL database connection timeout")
                ;

        AddCustomOptions(pgOptions);

        boost::program_options::options_description sqliteOptions{"SQLite options"};
        sqliteOptions.add_options()
                ("sqlite-in-memory", boost::program_options::value<bool>(&m_sqliteInMemory)->default_value(true),
                        "SQLite in-memory database")
                ("sqlite-dir", boost::program_options::value<std::string>(&m_sqliteDir)->default_value("."),
                        "SQLite database dir")
                ;

        AddCustomOptions(sqliteOptions);
    }

private:
    std::string m_type;
    std::string m_dbPrefix;
    bool m_cleanResult;

    std::string m_pgHost;
    std::uint16_t m_pgPort;
    std::string m_pgUser;
    std::string m_pgPassword;
    std::uint32_t m_pgConnectionTimeout;

    bool m_sqliteInMemory;
    std::string m_sqliteDir;

    std::string GenerateDbName() const
    {
        auto name = Mif::Common::UuidGenerator{}.Generate();
        boost::algorithm::erase_all(name, "-");

        if (!m_dbPrefix.empty())
            name = m_dbPrefix + "_" + name;

        return name;
    }

    // Mif.Application.Application
    virtual void OnStart() override final
    {
        if (m_type == "postgres")
            DemoPostgreSQL();
        else if (m_type == "sqlite")
            DemoSQLite();
        else
            throw std::invalid_argument{"Type \"" + m_type + "\" not supported."};
    }

    void ShowData(Mif::Db::IConnectionPtr connection)
    {
        // Run a parametrized query
        auto statement = connection->CreateStatement(
                "select * from test "
                "where id >= $1 and id <= $2 "
                "order by id;"
            );
        auto recordset = statement->Execute({"5", "7"});

        auto const count = recordset->GetFieldsCount();
        MIF_LOG(Info) << "Fields count: " << count;
        for (std::size_t i = 0 ; i < count ; ++i)
            MIF_LOG(Info) << "\"" << recordset->GetFieldName(i) << "\" is the name of the field "  << i <<  ".";

        while (recordset->Read())
        {
            for (std::size_t i = 0 ; i < count ; ++i)
            {
                MIF_LOG(Info) << recordset->GetFieldName(i) << ": "
                        << (recordset->IsNull(i) ? std::string{"null"} : recordset->GetAsString(i));
            }
        }
    }

    void DemoPostgreSQL()
    {
        auto dbName = GenerateDbName();

        // Create database
        {
            MIF_LOG(Info) << "Create database \"" << dbName << "\"";
            auto connection = Mif::Service::Create<Mif::Db::Id::Service::PostgreSQL, Mif::Db::IConnection>(
                    m_pgHost, m_pgPort, m_pgUser, m_pgPassword, std::string{}, m_pgConnectionTimeout);
            connection->ExecuteDirect(
                    "CREATE DATABASE " + dbName + " WITH OWNER " + m_pgUser + ";"
                );
        }

        // Drop database on exit from method
        BOOST_SCOPE_EXIT(&m_pgHost, &m_pgPort, &m_pgUser, &m_pgPassword, &m_pgConnectionTimeout,
                &m_cleanResult, &dbName)
        {
            if (m_cleanResult)
            {
                try
                {
                    MIF_LOG(Info) << "Drop database \"" << dbName << "\"";
                    auto connection = Mif::Service::Create<Mif::Db::Id::Service::PostgreSQL, Mif::Db::IConnection>(
                            m_pgHost, m_pgPort, m_pgUser, m_pgPassword, std::string{}, m_pgConnectionTimeout);
                    connection->ExecuteDirect("DROP DATABASE " + dbName + ";");
                }
                catch (std::exception const &e)
                {
                    MIF_LOG(Warning) << "Failed to drop database \"" << dbName << "\" Error: " << e.what();
                }
            }
        }
        BOOST_SCOPE_EXIT_END

        // Connect to database
        MIF_LOG(Info) << "Connect ot database \"" << dbName << "\"";
        auto connection = Mif::Service::Create<Mif::Db::Id::Service::PostgreSQL, Mif::Db::IConnection>(
                m_pgHost, m_pgPort, m_pgUser, m_pgPassword, dbName, m_pgConnectionTimeout);

        Mif::Db::Transaction transaction{connection};

        // Create table 'test'
        MIF_LOG(Info) << "Create table 'test'";
        connection->ExecuteDirect(
                "create table test"
                "("
                "   id serial not null primary key,"
                "   key varchar not null,"
                "   value varchar"
                ");"
            );

        // Create index
        MIF_LOG(Info) << "Create index";
        connection->ExecuteDirect(
                "create unique index test_unique_key_index on test (key);"
            );

        // Fill table
        MIF_LOG(Info) << "Fill table";
        connection->ExecuteDirect(
                "insert into test (key, value) "
                "select 'key_' || t.i::text, 'value_' || t.i::text "
                "from generate_series(1, 10) as t(i);"
            );

        transaction.Commit();

        // Show data
        ShowData(connection);
    }

    void DemoSQLite()
    {
        std::string fileName;

        BOOST_SCOPE_EXIT(&m_cleanResult, &fileName)
        {
            if (m_cleanResult && !fileName.empty())
            {
                // Remove database file
                MIF_LOG(Info) << "Remove database file \"" << fileName << "\"";
                boost::filesystem::remove(fileName);
            }
        }
        BOOST_SCOPE_EXIT_END

        // Connect to database
        Mif::Db::IConnectionPtr connection;
        if (m_sqliteInMemory)
        {
            MIF_LOG(Info) << "Create in-memory database";
            connection = Mif::Service::Create<Mif::Db::Id::Service::SQLite, Mif::Db::IConnection>();
        }
        else
        {
            auto const path = boost::filesystem::absolute(m_sqliteDir).parent_path() / GenerateDbName();
            fileName = path.c_str();
            MIF_LOG(Info) << "Create or connect database from file \"" << fileName << "\"";
            connection = Mif::Service::Create<Mif::Db::Id::Service::SQLite, Mif::Db::IConnection>(fileName);
        }

        Mif::Db::Transaction transaction{connection};

        // Create table 'test'
        MIF_LOG(Info) << "Create table 'test'";
        connection->ExecuteDirect(
                "create table test"
                "("
                "   id integer primary key autoincrement,"
                "   key varchar not null,"
                "   value varchar"
                ");"
            );

        // Fill table
        MIF_LOG(Info) << "Fill table";
        for (int i = 1 ; i <= 10 ; ++i)
        {
            auto const index = std::to_string(i);
            connection->ExecuteDirect(
                    "insert into test (key, value) values ('key_" + index + "', 'value_" + index + "')"
                );
        }

        transaction.Commit();

        // Show data
        ShowData(connection);
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
