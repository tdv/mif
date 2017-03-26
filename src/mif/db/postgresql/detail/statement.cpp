//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// BOOST
#include <boost/algorithm/string.hpp>

// MIF
#include "mif/common/log.h"
#include "mif/common/uuid_generator.h"
#include "mif/service/make.h"

// THIS
#include "statement.h"
#include "recordset.h"

namespace Mif
{
    namespace Db
    {
        namespace PostgreSql
        {
            namespace Detail
            {

                Statement::Statement(PGconn *connection, Service::IService *holder,
                        std::string const &query)
                    : m_connection{connection}
                    , m_holder{holder}
                    , m_name{"mif_ps_" + Common::UuidGenerator{}.Generate()}
                    , m_query{query}
                {
                    if (!m_connection)
                        throw std::invalid_argument{"[Mif::Db::PostgreSql::Detail::Statement] Empty connection pointer."};

                    if (!m_holder)
                        throw std::invalid_argument{"[Mif::Db::PostgreSql::Detail::Statement] Empty connection holder pointer."};

                    if (m_query.empty())
                        throw std::invalid_argument{"[Mif::Db::PostgreSql::Detail::Statement] Empty query."};

                    boost::algorithm::erase_all(m_name, "-");

                    ResultPtr result{PQprepare(m_connection, m_name.c_str(), m_query.c_str(), 0, nullptr),
                            [] (PGresult *res) { if (res) PQclear(res); } };

                    if (!result)
                    {
                        throw std::runtime_error{"[Mif::Db::PostgreSql::Detail::Statement] "
                                "Failed to create prepared statement for query \"" + m_query + "\""};
                    }

                    if (PQresultStatus(result.get()) != PGRES_COMMAND_OK)
                    {
                        auto const *message = PQresultErrorMessage(result.get());

                        throw std::runtime_error{"[Mif::Db::PostgreSql::Detail::Statement] "
                                "Failed to create prepared statement for query \"" + m_query + "\". "
                                "Erro: " + std::string{message ? message : "unknown"}};
                    }
                }

                Statement::~Statement()
                {
                    try
                    {
                        std::string command{"DEALLOCATE " + m_name};
                        ResultPtr result{PQexec(m_connection, command.c_str()),
                                [] (PGresult *res) { if (res) PQclear(res); } };
                        if (!result)
                            throw std::runtime_error{"Failed to deallocate statement \"" + m_name + "\" for query \"" + m_query + "\""};
                        if (PQresultStatus(result.get()) != PGRES_COMMAND_OK)
                        {
                            auto const *message = PQresultErrorMessage(result.get());
                            throw std::runtime_error{"Failed to deallocate statement \"" + m_name + "\" for query \"" + m_query + "\". "
                                    "Error: " + std::string{message ? message : "unknown"}};
                        }
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Warning) << "[Mif::Db::PostgreSql::Detail::~Statement] " << e.what();
                    }
                }

                IRecordsetPtr Statement::Execute(Parameters const &parameters)
                {
                    return Service::Make<Recordset, IRecordset>(m_connection, this, m_name, parameters);
                }

            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Db
}   // namespace Mif
