//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// BOOST
#include <boost/scoped_array.hpp>

// THIS
#include "recordset.h"

namespace Mif
{
    namespace Db
    {
        namespace PostgreSql
        {
            namespace Detail
            {

                Recordset::Recordset(PGconn *connection, Service::IService *holder, std::string const &statementName)
                    : m_connection{connection}
                    , m_holder{holder}
                    , m_statementName{statementName}
                {
                    if (!m_connection)
                        throw std::invalid_argument{"[Mif::Db::PostgreSql::Detail::Recordset] Empty connection pointer."};

                    if (!m_holder)
                        throw std::invalid_argument{"[Mif::Db::PostgreSql::Detail::Recordset] Empty connection holder pointer."};

                    if (m_statementName.empty())
                        throw std::invalid_argument{"[Mif::Db::PostgreSql::Detail::Recordset] Empty query."};

                    m_result.reset(PQexecPrepared(m_connection, m_statementName.c_str(), 0, nullptr, nullptr, nullptr, 0));

                    if (!m_result)
                        throw std::runtime_error{"[Mif::Db::PostgreSql::Detail::Recordset] Failed to open recordset."};

                    auto const status = PQresultStatus(m_result.get());
                    if (status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK)
                    {
                        auto const *message = PQresultErrorMessage(m_result.get());
                        throw std::runtime_error{"[Mif::Db::PostgreSql::Detail::Recordset] Failed to open recordset. "
                                "Error: " + std::string{message ? message : "unknown"}};
                    }
                }

                Recordset::~Recordset()
                {
                }

            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Db
}   // namespace Mif
