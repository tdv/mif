//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// STD
#include <functional>
#include <stdexcept>
#include <utility>

// MIF
#include "mif/common/log.h"
#include "mif/service/make.h"

// THIS
#include "statement.h"
#include "recordset.h"

namespace Mif
{
    namespace Db
    {
        namespace SQLite
        {
            namespace Detail
            {

                Statement::Statement(sqlite3 *connection, Service::IService *holder,
                        std::string const &query)
                    : m_connection{connection}
                    , m_holder{holder}
                    , m_query{query}
                {
                    if (!m_connection)
                        throw std::invalid_argument{"[Mif::Db::SQLite::Detail::Statement] Empty connection pointer."};

                    if (!m_holder)
                        throw std::invalid_argument{"[Mif::Db::SQLite::Detail::Statement] Empty connection holder pointer."};

                    if (m_query.empty())
                        throw std::invalid_argument{"[Mif::Db::SQLite::Detail::Statement] Empty query."};

                    /*if (PQresultStatus(result.get()) != PGRES_COMMAND_OK)
                    {
                        auto const *message = PQresultErrorMessage(result.get());

                        throw std::runtime_error{"[Mif::Db::SQLite::Detail::Statement] "
                                "Failed to create prepared statement for query \"" + m_query + "\". "
                                "Erro: " + std::string{message ? message : "unknown"}};
                    }*/
                }

                IRecordsetPtr Statement::Execute(Parameters const &parameters)
                {
                    sqlite3_stmt *stmt = nullptr;
                    auto const res = sqlite3_prepare_v2(m_connection, m_query.c_str(), m_query.length() + 1, &stmt, nullptr);

                    using StatementPtr = std::unique_ptr<sqlite3_stmt, std::function<void (sqlite3_stmt *)>>;
                    auto const needToCleanBindings = !parameters.empty();
                    StatementPtr statement{stmt, [needToCleanBindings] (sqlite3_stmt *s)
                            {
                                if (!s)
                                    return;

                                if (needToCleanBindings && sqlite3_clear_bindings(s) != SQLITE_OK)
                                {
                                    MIF_LOG(Warning) << "[Mif::Db::SQLite::Detail::Statement::Execute] "
                                                     << "Failed to clean binded parameters in the statement.";
                                }

                                if (sqlite3_finalize(s) != SQLITE_OK)
                                {
                                    MIF_LOG(Warning) << "[Mif::Db::SQLite::Detail::Statement::Execute] "
                                                     << "Failed to close statement.";
                                }
                            }
                        };

                    if (res != SQLITE_OK)
                    {
                        throw std::runtime_error{"[Mif::Db::SQLite::Detail::Statement::Execute] "
                                "Failed to create prepared statement for query \"" + m_query + "\""};
                    }

                    int index = 1;
                    for (auto const &prm : parameters)
                    {
                        if (prm.empty())
                        {
                            if (sqlite3_bind_null(statement.get(), index) != SQLITE_OK)
                            {
                                throw std::runtime_error{"[Mif::Db::SQLite::Detail::Statement::Execute] "
                                        "Failed to bind null parameter with index " + std::to_string(index) +
                                        " to statement for query \"" + m_query + "\""};
                            }
                        }
                        else
                        {
                            if (sqlite3_bind_text(statement.get(), index, prm.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
                            {
                                throw std::runtime_error{"[Mif::Db::SQLite::Detail::Statement::Execute] "
                                        "Failed to bind \"" + prm + "\" parameter with index " + std::to_string(index) +
                                        " to statement for query \"" + m_query + "\""};
                            }
                        }

                        ++index;
                    }

                    return Service::Make<Recordset, IRecordset>(this, std::move(statement));
                }

            }   // namespace Detail
        }   // namespace SQLite
    }   // namespace Db
}   // namespace Mif
