//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <cstdint>
#include <stdexcept>
#include <string>

// MIF
#include <mif/common/log.h>
#include <mif/db/iconnection.h>
#include <mif/db/id/service.h>
#include <mif/service/create.h>

int main(/*int argc, char const **argv*/)
{
    try
    {
        std::string const host = "localhost";
        std::uint16_t port = 5432;
        std::string const user = "postgres";
        std::string const password = "";
        std::string const db = "mif_orm_test";
        std::uint32_t connectionTimeout = 10;

        auto connection = Mif::Service::Create<Mif::Db::Id::Service::PostgreSQL, Mif::Db::IConnection>(
                host, port, user, password, db, connectionTimeout);

        connection->ExecuteDirect("BEGIN;");
        connection->ExecuteDirect(
                "create table test "
                "("
                "test_id bigserial not null primary key,"
                "key varchar not null,"
                "value varchar"
                ")"
            );
        connection->ExecuteDirect(
                "create unique index test_unique_key_index on test (key);"
            );
        connection->ExecuteDirect(
                "insert into test (key, value) "
                "select 'key_' || t.i::text, 'value_' || t.i::text "
                "from generate_series(1, 10) as t(i);"
            );

        auto statement = connection->CreateStatement(
                "select * from test "
                "where test_id >= $1 and test_id <= $2 "
                "order by test_id;"
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

        connection->ExecuteDirect(
                "drop table test;"
            );
        connection->ExecuteDirect("COMMIT;");
    }
    catch (std::exception const &e)
    {
        MIF_LOG(Error) << e.what();
    }
    return 0;
}
