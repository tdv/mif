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

                     auto const count = PQnfields(m_result.get());
                     if (count < 0)
                     {
                         throw std::runtime_error{"[Mif::Db::PostgreSql::Detail::Recordset] "
                                 "Failed to get fields count."};
                     }

                     m_fieldsCount = static_cast<std::size_t>(count);
                }

                void Recordset::CheckIndex(std::size_t index) const
                {
                    if (!m_fieldsCount)
                    {
                        throw std::invalid_argument{"[Mif::Db::PostgreSql::Detail::Recordset::CheckIndex] "
                            "Failed to get value. There are no columns in the recordset."};
                    }

                    if (index >= m_fieldsCount)
                    {
                        throw std::invalid_argument{"[Mif::Db::PostgreSql::Detail::Recordset::CheckIndex] "
                            "Failed to get value. Index " + std::to_string(index) + " is out of range "
                            "[0 ... " + std::to_string(m_fieldsCount) + "]."};
                    }
                }

                bool Recordset::Read()
                {
                    if (!m_hasNext)
                        return false;

                    if (PQntuples(m_result.get()) <= m_currentRow + 1)
                    {
                        m_hasNext = false;
                        return false;
                    }

                    ++m_currentRow;
                    return true;
                }

                std::size_t Recordset::GetFieldsCount() const
                {
                    return m_fieldsCount;
                }

                bool Recordset::IsNull(std::size_t index) const
                {
                    CheckIndex(index);
                    return !!PQgetisnull(m_result.get(), m_currentRow, static_cast<int>(index));
                }

                std::string Recordset::GetFieldName(std::size_t index) const
                {
                    auto const *name = PQfname(m_result.get(), static_cast<int>(index));
                    if (!name)
                    {
                        throw std::runtime_error{"[Mif::Db::PostgreSql::Detail::Recordset::GetFieldName] "
                                "Failed to get " + std::to_string(index) + " field name."};
                    }

                    return name;
                }

                std::string Recordset::GetAsString(std::size_t index) const
                {
                    if (IsNull(index))
                    {
                        throw std::logic_error{"[Mif::Db::PostgreSql::Detail::Recordset::GetAsString] "
                            "Failed to get " + std::to_string(index) + " field value from null."};
                    }

                    auto const *value = PQgetvalue(m_result.get(), m_currentRow, static_cast<int>(index));

                    if (!value)
                    {
                        throw std::runtime_error{"[Mif::Db::PostgreSql::Detail::Recordset::GetAsString] "
                            "Failed to get " + std::to_string(index) + " field value."};
                    }

                    return value;
                }

                std::int32_t Recordset::GetAsInt32(std::size_t index) const
                try
                {
                    return static_cast<std::int32_t>(std::stoi(GetAsString(index)));
                }
                catch (std::exception const &e)
                {
                    throw std::runtime_error{"[Mif::Db::PostgreSql::Detail::Recordset::GetAsInt32] "
                        "Failed to get " + std::to_string(index) + " field value. Error: " + std::string{e.what()}};
                }

                std::int64_t Recordset::GetAsInt64(std::size_t index) const
                try
                {
                    return static_cast<std::int64_t>(std::stoll(GetAsString(index)));
                }
                catch (std::exception const &e)
                {
                    throw std::runtime_error{"[Mif::Db::PostgreSql::Detail::Recordset::GetAsInt64] "
                        "Failed to get " + std::to_string(index) + " field value. Error: " + std::string{e.what()}};
                }

                double Recordset::GetAsDouble(std::size_t index) const
                try
                {
                    return (std::stod(GetAsString(index)));
                }
                catch (std::exception const &e)
                {
                    throw std::runtime_error{"[Mif::Db::PostgreSql::Detail::Recordset::GetAsDouble] "
                        "Failed to get " + std::to_string(index) + " field value. Error: " + std::string{e.what()}};
                }

            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Db
}   // namespace Mif
