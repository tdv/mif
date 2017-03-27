//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

// BOOST
#include <boost/scope_exit.hpp>

// SQLITE
#include <sqlite3.h>

// MIF
#include "mif/common/log.h"
#include "mif/db/iconnection.h"
#include "mif/db/id/service.h"
#include "mif/service/creator.h"
#include "mif/service/make.h"

// THIS
#include "detail/statement.h"

namespace Mif
{
    namespace Db
    {
        namespace SQLite
        {
            namespace
            {

                class Connection
                    : public Service::Inherit<IConnection>
                {
                public:
                    Connection()
                        : Connection(":memory:")
                    {
                    }

                    Connection(std::string fileName)
                    {
                        if (fileName.empty())
                            throw std::invalid_argument{"[Mif::Db::SQLIte::Connection] Empty file name."};

                        sqlite3 *connection = nullptr;
                        auto const res = sqlite3_open_v2(fileName.c_str(), &connection,
                                SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                                (fileName == ":memory:" ? SQLITE_OPEN_MEMORY : 0), nullptr);
                        m_connection.reset(connection);

                        if (res != SQLITE_OK)
                        {
                            char const *message = nullptr;
                            if (m_connection)
                                message = sqlite3_errstr(res);
                            else
                                message = sqlite3_errmsg(m_connection.get());
                            throw std::runtime_error{"[Mif::Db::SQLite::Connection] Failed to open in-memory database. "
                                    "Error: " + std::string{message ? message : "unknown"}};
                        }
                    }

                private:
                    using ConnectionPtr = std::unique_ptr<sqlite3, std::function<void (sqlite3 *)>>;
                    ConnectionPtr m_connection{nullptr, [] (sqlite3 *conn)
                            {
                                if (!conn)
                                    return;
                                if (sqlite3_close_v2(conn) != SQLITE_OK)
                                    MIF_LOG(Warning) << "[Mif::Db::SQLite::Connection] Failed to close connection.";
                            }
                        };

                    // IConnection
                    virtual void ExecuteDirect(std::string const &query) override final
                    {
                        if (query.empty())
                            throw std::invalid_argument{"[Mif::Db::SQLite::Connection::ExecuteDirect] Empry query string."};

                        char *message = nullptr;

                        BOOST_SCOPE_EXIT(message)
                        {
                            if (message)
                                sqlite3_free(message);
                        }
                        BOOST_SCOPE_EXIT_END

                        auto const res = sqlite3_exec(m_connection.get(), query.c_str(), nullptr, nullptr, &message);

                        if (res != SQLITE_OK)
                        {
                            throw std::runtime_error{"[Mif::Db::SQLite::Connection::ExecuteDirect] "
                                    "Failed to execute query \"" + query + "\" Error: " + std::string{message ? message : "unknown"}};
                        }

                        (void)query;
                    }

                    virtual IStatementPtr CreateStatement(std::string const &query) override final
                    {
                        return Service::Make<Detail::Statement, IStatement>(m_connection.get(), this, query);
                    }
                };
            }   // namespace
        }   // namespace SQLite
    }   // namespace Db
}   // namespace Mif

MIF_SERVICE_CREATOR
(
    Mif::Db::Id::Service::SQLite,
    Mif::Db::SQLite::Connection
)

MIF_SERVICE_CREATOR
(
    Mif::Db::Id::Service::SQLite,
    Mif::Db::SQLite::Connection,
    std::string
)
