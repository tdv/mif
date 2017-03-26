//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <sstream>

// LIBPQ
#include <libpq-fe.h>

// MIF
#include "mif/db/iconnection.h"
#include "mif/db/id/service.h"
#include "mif/service/creator.h"

// THIS
#include "detail/statement.h"

namespace Mif
{
    namespace Db
    {
        namespace PostgreSql
        {
            namespace
            {

                class Connection
                    : public Service::Inherit<IConnection>
                {
                public:
                    Connection(std::string const &connectionString)
                    {
                        if (connectionString.empty())
                            throw std::invalid_argument{"[Mif::Db::PostgreSql::Connection] Empty connection string."};

                        m_connection.reset(PQconnectdb(connectionString.c_str()));

                        if (PQstatus(m_connection.get()) == CONNECTION_OK)
                            return;

                        auto const *message = PQerrorMessage(m_connection.get());

                        throw std::runtime_error{"[Mif::Db::PostgreSql::Connection] Failed to open connection. "
                                "Error: " + std::string{message ? message : "unknown"}};
                    }

                    Connection(std::string const &host, std::uint16_t port, std::string const &user, std::string const &password,
                            std::string const &db, std::uint32_t connectionTimeout)
                        : Connection{
                                "host='" + host +  "' "
                                "port='" + std::to_string(port) + "' "
                                "user='" + user + "' "
                                "password='" + password + "' "
                                "dbname='" + db + "' "
                                "connect_timeout='" + std::to_string(connectionTimeout) + "' "
                                "sslmode='disable'"
                            }
                    {
                    }

                private:
                    using ConnectionPtr = std::unique_ptr<PGconn, decltype(&PQfinish)>;
                    ConnectionPtr m_connection{nullptr, [] (PGconn *conn) { if (conn) PQfinish(conn); } };

                    // IConnection
                    virtual void ExecuteDirect(std::string const &query) override final
                    {
                        if (query.empty())
                            throw std::invalid_argument{"[Mif::Db::PostgreSql::Connection::ExecuteDirect] Empry query string."};

                        Detail::Statement::ResultPtr result{PQexec(m_connection.get(), query.c_str()),
                                [] (PGresult *res) { if (res) PQclear(res); } };

                        if (PQresultStatus(result.get()) == PGRES_COMMAND_OK)
                            return;

                        auto const *message = PQerrorMessage(m_connection.get());
                        throw std::runtime_error{"[Mif::Db::PostgreSql::Connection::ExecuteDirect] "
                                "Failed to execute query \"" + query + "\" Error: " + std::string{message ? message : "unknown"}};
                    }

                    virtual IStatementPtr CreateStatement(std::string const &query) override final
                    {
                        return Service::Make<Detail::Statement, IStatement>(m_connection.get(), this, query);
                    }
                };

            }   // namespace
        }   // namespace PostgreSql
    }   // namespace Db
}   // namespace Mif

MIF_SERVICE_CREATOR
(
    Mif::Db::Id::Service::PostgreSQL,
    Mif::Db::PostgreSql::Connection,
    std::string,
    std::uint16_t,
    std::string,
    std::string,
    std::string,
    std::uint32_t
)

MIF_SERVICE_CREATOR
(
    Mif::Db::Id::Service::PostgreSQL,
    Mif::Db::PostgreSql::Connection,
    std::string
)
