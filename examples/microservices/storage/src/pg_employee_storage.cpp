//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// MIF
#include <mif/application/iconfig.h>
#include <mif/common/log.h>
#include <mif/db/iconnection_pool.h>
#include <mif/db/id/service.h>
#include <mif/db/transaction.h>
#include <mif/reflection/reflection.h>
#include <mif/service/creator.h>

// COMMON
#include "common/id/service.h"
#include "common/interface/iemployee_storage.h"
#include "common/data/meta/data.h"

namespace Storage
{
    namespace Detail
    {
        namespace
        {

            class PGEmployeeStorage
                : public Mif::Service::Inherit<Common::IEmployeeStorage>
            {
            public:
                PGEmployeeStorage(Mif::Application::IConfigPtr config)
                {
                    if (!config)
                        throw std::invalid_argument{"[Storage::Detail::PGEmployeeStorage] Empty config ptr."};

                    m_connections = Mif::Service::Create
                            <Mif::Db::Id::Service::PostgresConnectionPool, Mif::Db::IConnectionPool>
                            (config);
                }

            private:
                Mif::Db::IConnectionPoolPtr m_connections;

                // IEmployeeStorage
                virtual Common::Data::ID Create(Common::Data::Employee const &employee) override final
                {
                    Common::Data::ID id;

                    try
                    {
                        auto connection = m_connections->GetConnection();
                        Mif::Db::Transaction tr{connection};

                        auto statement = connection->CreateStatement(
                                "INSERT INTO employees (name, last_name, age, email, position, rate) "
                                "VALUES ($1::text, $2::text, $3::integer, $4::text, $5::\"Position\", $6::numeric(10, 2))"
                                "RETURNING employee_id ; "
                            );

                        auto result = statement->Execute({employee.name, employee.lastName, std::to_string(employee.age),
                                employee.email, Mif::Reflection::ToString(employee.position), std::to_string(employee.rate)});

                        if (!result->Read())
                            throw std::runtime_error{"Failed to create employee."};

                        id = result->GetAsString(0);

                        tr.Commit();
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Error) << "[Storage::Detail::PGEmployeeStorage::Create] "
                                       << "Failed to create new record for employee. "
                                       << "Error: " << e.what();

                        throw std::runtime_error{"Failed to create new record for employee."};
                    }

                    return id;
                }

                virtual Common::Data::Employee Read(Common::Data::ID const &id) const override final
                {
                    Common::Data::Employee employee;

                    try
                    {
                        auto connection = m_connections->GetConnection();
                        Mif::Db::Transaction tr{connection};

                        auto statement = connection->CreateStatement(
                                "SELECT name, last_name, age, email, position, rate "
                                "FROM employees "
                                "WHERE employee_id = $1::bigint ;"
                            );

                        auto result = statement->Execute({id});

                        if (!result->Read())
                            throw std::invalid_argument{"Employee with id \"" + id + "\" not found."};

                        std::size_t index = 0;

                        employee.name = result->GetAsString(index++);
                        employee.lastName = result->GetAsString(index++);
                        employee.age = static_cast<std::uint32_t>(result->GetAsInt32(index++));
                        employee.email = result->GetAsString(index++);
                        employee.position = Mif::Reflection::FromString<Common::Data::Position>(result->GetAsString(index++));
                        employee.rate = result->GetAsDouble(index++);
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Error) << "[Storage::Detail::PGEmployeeStorage::Read] "
                                       << "Failed to read employee with id \"" << id << "\". "
                                       << "Error: " << e.what();

                        throw std::runtime_error{"Failed to read employee with id \"" + id + "\"."};
                    }

                    return employee;
                }

                virtual void Update(Common::Data::ID const &id, Common::Data::Employee const &employee) override final
                {
                    try
                    {
                        auto connection = m_connections->GetConnection();
                        Mif::Db::Transaction tr{connection};

                        auto statement = connection->CreateStatement(
                                "UPDATE employees "
                                "SET name=$1::text, last_name=$2::text, age=$3::integer, email=$4::text, "
                                    "position=$5::\"Position\", rate=$6::numeric(10, 2)  "
                                "WHERE employee_id = $7::bigint "
                                "RETURNING employee_id ; "
                            );

                        auto result = statement->Execute({employee.name, employee.lastName, std::to_string(employee.age),
                                employee.email, Mif::Reflection::ToString(employee.position), std::to_string(employee.rate), id});

                        if (!result->Read())
                            throw std::invalid_argument{"Employee with id \"" + id + "\" not found."};


                        tr.Commit();
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Error) << "[Storage::Detail::PGEmployeeStorage::Update] "
                                       << "Failed to update employee with id \"" << id << "\". "
                                       << "Error: " << e.what();

                        throw std::runtime_error{"Failed to update employee with id \"" + id + "\"."};
                    }
                }

                virtual void Delete(Common::Data::ID const &id) override final
                {
                    try
                    {
                        auto connection = m_connections->GetConnection();
                        Mif::Db::Transaction tr{connection};

                        auto statement = connection->CreateStatement(
                                "DELETE FROM employees "
                                "WHERE employee_id = $1::bigint "
                                "RETURNING employee_id ; "
                            );

                        auto result = statement->Execute({id});

                        if (!result->Read())
                            throw std::invalid_argument{"Employee with id \"" + id + "\" not found."};


                        tr.Commit();
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Error) << "[Storage::Detail::PGEmployeeStorage::Delete] "
                                       << "Failed to delete employee with id \"" << id << "\". "
                                       << "Error: " << e.what();

                        throw std::runtime_error{"Failed to delete employee with id \"" + id + "\"."};
                    }
                }

                virtual Common::Data::Employees List(std::size_t offset, std::size_t limit) const override final
                {
                    Common::Data::Employees employees;

                    try
                    {
                        auto connection = m_connections->GetConnection();
                        Mif::Db::Transaction tr{connection};

                        std::string sql = "SELECT name, last_name, age, email, position, rate, employee_id "
                                "FROM employees ";


                        Mif::Db::IStatement::Parameters params;

                        if (offset != std::numeric_limits<std::size_t>::max())
                        {
                            sql += "OFFSET $1::integer ";
                            params.push_back(std::to_string(offset));
                        }

                        if (limit != std::numeric_limits<std::size_t>::max())
                        {
                            auto const index = offset == std::numeric_limits<std::size_t>::max() ? 1 : 2;
                            sql += "LIMIT $" + std::to_string(index) + "::integer ";
                            params.push_back(std::to_string(limit));
                        }

                        sql += ";";

                        auto statement = connection->CreateStatement(sql);
                        auto result = statement->Execute(params);

                        while (result->Read())
                        {
                            std::size_t index = 0;

                            Common::Data::Employee employee;

                            employee.name = result->GetAsString(index++);
                            employee.lastName = result->GetAsString(index++);
                            employee.age = static_cast<std::uint32_t>(result->GetAsInt32(index++));
                            employee.email = result->GetAsString(index++);
                            employee.position = Mif::Reflection::FromString<Common::Data::Position>(result->GetAsString(index++));
                            employee.rate = result->GetAsDouble(index++);

                            employees.emplace(result->GetAsString(index), std::move(employee));
                        }
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Error) << "[Storage::Detail::PGEmployeeStorage::Delete] "
                                       << "Failed to get employees list. "
                                       << "Error: " << e.what();

                        throw std::runtime_error{"Failed to get employees list."};
                    }

                    return employees;
                }
            };

        }   // namespace
    }   // namespace Detail
}   // namespace Storage

MIF_SERVICE_CREATOR
(
    ::Common::Service::Id::PGEmployeeStorage,
    ::Storage::Detail::PGEmployeeStorage,
    Mif::Application::IConfigPtr
)
