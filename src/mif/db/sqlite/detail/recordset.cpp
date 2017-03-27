//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>
#include <utility>

// THIS
#include "recordset.h"

namespace Mif
{
    namespace Db
    {
        namespace SQLite
        {
            namespace Detail
            {

                Recordset::Recordset(Service::IService *holder, StatementPtr statement)
                    : m_holder{holder}
                    , m_statement{std::move(statement)}
                {
                    if (!m_holder)
                    throw std::invalid_argument{"[Mif::Db::SQLite::Detail::Recordset] Empty connection holder pointer."};

                    if (!m_statement)
                        throw std::invalid_argument{"[Mif::Db::SQLite::Detail::Recordset] Empty statement pointer."};

                    auto const count = sqlite3_column_count(m_statement.get());
                    if (count < 0)
                    {
                        throw std::runtime_error{"[Mif::Db::SQLite::Detail::Recordset] "
                                "Failed to get fields count."};
                    }

                     m_fieldsCount = static_cast<std::size_t>(count);

                     for (std::size_t i = 0 ; i < m_fieldsCount ; ++i)
                     {
                         auto const *name = sqlite3_column_name(m_statement.get(), i);
                         if (!name)
                         {
                             throw std::runtime_error{"[Mif::Db::SQLite::Detail::Recordset] "
                                     "Failed to get " + std::to_string(i) + " field name."};
                         }
                         m_fieldsNames.emplace(i, name);
                         m_fieldsIndexes.emplace(name, i);
                     }
                }

                void Recordset::CheckIndex(std::size_t index) const
                {
                    if (!m_fieldsCount)
                    {
                        throw std::invalid_argument{"[Mif::Db::SQLite::Detail::Recordset::CheckIndex] "
                            "Failed to get value. There are no columns in the recordset."};
                    }

                    if (index >= m_fieldsCount)
                    {
                        throw std::invalid_argument{"[Mif::Db::SQLite::Detail::Recordset::CheckIndex] "
                            "Failed to get value. Index " + std::to_string(index) + " is out of range "
                            "[0 ... " + std::to_string(m_fieldsCount) + "]."};
                    }
                }

                bool Recordset::Read()
                {
                    if (m_finished)
                        return false;

                    auto const res = sqlite3_step(m_statement.get());
                    switch (res)
                    {
                    case SQLITE_ROW :
                        return true;
                    case SQLITE_DONE:
                        m_finished = true;
                        return false;
                    default:
                        break;
                    }

                    throw std::runtime_error{"[Mif::Db::SQLite::Detail::Recordset::Read] Failed to read data."};
                }

                std::size_t Recordset::GetFieldsCount() const
                {
                    return m_fieldsCount;
                }

                bool Recordset::IsNull(std::size_t index) const
                {
                    CheckIndex(index);
                    return sqlite3_column_type(m_statement.get(), index) == SQLITE_NULL;
                }

                std::string Recordset::GetFieldName(std::size_t index) const
                {
                    CheckIndex(index);

                    auto const iter = m_fieldsNames.find(index);
                    if (iter == std::end(m_fieldsNames))
                    {
                        throw std::invalid_argument{"[Mif::Db::SQLite::Detail::Recordset::GetFieldName] "
                                "Failed to get " + std::to_string(index) + " field name."};
                    }

                    return iter->second;
                }

                std::size_t Recordset::GetFieldIndex(std::string const &name) const
                {
                    auto const iter = m_fieldsIndexes.find(name);
                    if (iter == std::end(m_fieldsIndexes))
                    {
                        throw std::invalid_argument{"[Mif::Db::SQLite::Detail::Recordset::GetFieldIndex] "
                                "Failed to get \"" + name + "\" field index."};
                    }

                    return iter->second;
                }

                std::string Recordset::GetAsString(std::size_t index) const
                {
                    if (IsNull(index))
                    {
                        throw std::logic_error{"[Mif::Db::SQLite::Detail::Recordset::GetAsString] "
                            "Failed to get " + std::to_string(index) + " field value from null."};
                    }

                    auto const *text = sqlite3_column_text(m_statement.get(), index);

                    if (!text)
                    {
                        throw std::runtime_error{"[Mif::Db::SQLite::Detail::Recordset::GetAsString] "
                            "Failed to get " + std::to_string(index) + " field value."};
                    }

                    return reinterpret_cast<std::string::value_type const *>(text);
                }

                std::int32_t Recordset::GetAsInt32(std::size_t index) const
                try
                {
                    if (IsNull(index))
                        throw std::logic_error{"Failed to get field value from null."};

                    return static_cast<std::int32_t>(sqlite3_column_int(m_statement.get(), index));
                }
                catch (std::exception const &e)
                {
                    throw std::runtime_error{"[Mif::Db::SQLite::Detail::Recordset::GetAsInt32] "
                        "Failed to get " + std::to_string(index) + " field value. Error: " + std::string{e.what()}};
                }

                std::int64_t Recordset::GetAsInt64(std::size_t index) const
                try
                {
                    if (IsNull(index))
                        throw std::logic_error{"Failed to get field value from null."};

                    return static_cast<std::int64_t>(sqlite3_column_int64(m_statement.get(), index));
                }
                catch (std::exception const &e)
                {
                    throw std::runtime_error{"[Mif::Db::SQLite::Detail::Recordset::GetAsInt64] "
                        "Failed to get " + std::to_string(index) + " field value. Error: " + std::string{e.what()}};
                }

                double Recordset::GetAsDouble(std::size_t index) const
                try
                {
                    if (IsNull(index))
                        throw std::logic_error{"Failed to get field value from null."};

                    return sqlite3_column_double(m_statement.get(), index);
                }
                catch (std::exception const &e)
                {
                    throw std::runtime_error{"[Mif::Db::SQLite::Detail::Recordset::GetAsDouble] "
                        "Failed to get " + std::to_string(index) + " field value. Error: " + std::string{e.what()}};
                }

            }   // namespace Detail
        }   // namespace SQLite
    }   // namespace Db
}   // namespace Mif
