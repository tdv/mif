//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// BOOST
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// MIF
#include <mif/common/log.h>
#include <mif/db/id/service.h>
#include <mif/db/transaction.h>
#include <mif/reflection/reflection.h>
#include <mif/service/create.h>

// THIS
#include "employee_service.h"
#include "data/meta/data.h"

namespace Service
{

    EmployeeService::EmployeeService(std::string const &pathPrefix, Mif::Application::IConfigPtr dbConfig)
    {
        // Handlers
        AddHandler(pathPrefix + "/create", this, &EmployeeService::Create);
        AddHandler(pathPrefix + "/read", this, &EmployeeService::Read);
        AddHandler(pathPrefix + "/update", this, &EmployeeService::Update);
        AddHandler(pathPrefix + "/delete", this, &EmployeeService::Delete);

        AddHandler(pathPrefix + "/list", this, &EmployeeService::List);

        m_connections = Mif::Service::Create
                <Mif::Db::Id::Service::PostgresConnectionPool, Mif::Db::IConnectionPool>
                (dbConfig);
    }

    Data::Response::Info EmployeeService::CreateMeta(std::uint32_t code, std::string const &message) const
    {
        Data::Response::Info meta;
        meta.timestamp = boost::posix_time::to_iso_string(boost::posix_time::second_clock::universal_time());
        meta.node = boost::asio::ip::host_name();
        meta.status.code = code;
        meta.status.message = message;
        return meta;
    }

    std::string EmployeeService::FormatExceptionMessage(std::string const &message) const
    {
        try
        {
            MIF_LOG(Warning) << "[EmployeeService] Failed to process request. Error: " << message;

            Data::Response::Void response;
            response.meta = CreateMeta(-1, message);
            return Mif::Serialization::Json::Serialize(response).data();
        }
        catch (std::exception const &e)
        {
            MIF_LOG(Error) << "[EmployeeService] Failed to format message. Error: " << e.what() << " Message: " << message;
        }
        return message;
    }

    EmployeeService::ResultSerializer
    EmployeeService::Create(InputContent<Data::Employee> const &employee)
    {
        if (!employee)
            throw std::invalid_argument{"No data."};

        auto const &data = employee.Get();

        auto connection = m_connections->GetConnection();
        Mif::Db::Transaction tr{connection};

        auto statement = connection->CreateStatement(
                "INSERT INTO employees (name, last_name, age, email, position, rate) "
                "VALUES ($1::text, $2::text, $3::integer, $4::text, $5::\"Position\", $6::numeric(10, 2))"
                "RETURNING employee_id ; "
            );

        auto result = statement->Execute({data.name, data.lastName, std::to_string(data.age),
                data.email, Mif::Reflection::ToString(data.position), std::to_string(data.rate)});

        if (!result->Read())
            throw std::runtime_error{"[Service::EmployeeService::Create] Failed to create employee."};

        Data::Response::Id response;

        response.meta = CreateMeta();
        response.data = result->GetAsString(0);

        tr.Commit();

        return response;
    }

    EmployeeService::ResultSerializer
    EmployeeService::Read(Prm<Data::ID, Name("id")> const &id)
    {
        if (!id || id.Get().empty())
            throw std::invalid_argument{"Id must not be empty."};

        auto connection = m_connections->GetConnection();
        Mif::Db::Transaction tr{connection};

        auto statement = connection->CreateStatement(
                "SELECT name, last_name, age, email, position, rate "
                "FROM employees "
                "WHERE employee_id = $1::bigint ;"
            );

        auto result = statement->Execute({id.Get()});

        if (!result->Read())
            throw std::invalid_argument{"[Service::EmployeeService::Read] Employee with id \"" + id.Get() + "\" not found."};

        Data::Response::Employee response;

        response.meta = CreateMeta();

        std::size_t index = 0;

        response.data.name = result->GetAsString(index++);
        response.data.lastName = result->GetAsString(index++);
        response.data.age = static_cast<std::uint32_t>(result->GetAsInt32(index++));
        response.data.email = result->GetAsString(index++);
        response.data.position = Mif::Reflection::FromString<Data::Position>(result->GetAsString(index++));
        response.data.rate = result->GetAsDouble(index++);

        return response;
    }

    EmployeeService::ResultSerializer
    EmployeeService::Update(Prm<Data::ID, Name("id")> const &id,
                            InputContent<Data::Employee> const &employee)
    {
        if (!id || id.Get().empty())
            throw std::invalid_argument{"Id must not be empty."};

        if (!employee)
            throw std::invalid_argument{"No data."};

        auto const &data = employee.Get();

        auto connection = m_connections->GetConnection();
        Mif::Db::Transaction tr{connection};

        auto statement = connection->CreateStatement(
                "UPDATE employees "
                "SET name=$1::text, last_name=$2::text, age=$3::integer, email=$4::text, "
                    "position=$5::\"Position\", rate=$6::numeric(10, 2)  "
                "WHERE employee_id = $7::bigint "
                "RETURNING employee_id ; "
            );

        auto result = statement->Execute({data.name, data.lastName, std::to_string(data.age),
                data.email, Mif::Reflection::ToString(data.position), std::to_string(data.rate), id.Get()});

        if (!result->Read())
            throw std::invalid_argument{"[Service::EmployeeService::Update] Employee with id \"" + id.Get() + "\" not found."};

        tr.Commit();

        Data::Response::Void response;

        response.meta = CreateMeta();

        return response;
    }

    EmployeeService::ResultSerializer
    EmployeeService::Delete(Prm<Data::ID, Name("id")> const &id)
    {
        if (!id || id.Get().empty())
            throw std::invalid_argument{"Id must not be empty."};

        auto connection = m_connections->GetConnection();
        Mif::Db::Transaction tr{connection};

        auto statement = connection->CreateStatement(
                "DELETE FROM employees "
                "WHERE employee_id = $1::bigint "
                "RETURNING employee_id ; "
            );

        auto result = statement->Execute({id.Get()});

        if (!result->Read())
            throw std::invalid_argument{"[Service::EmployeeService::Delete] Employee with id \"" + id.Get() + "\" not found."};

        tr.Commit();

        Data::Response::Void response;

        response.meta = CreateMeta();

        return response;
    }

    EmployeeService::ResultSerializer
    EmployeeService::List(Prm<std::size_t, Name("offset")> const &offset,
                          Prm<std::size_t, Name("limit")> const &limit)
    {
        auto connection = m_connections->GetConnection();
        Mif::Db::Transaction tr{connection};

        std::string sql = "SELECT name, last_name, age, email, position, rate, employee_id "
                "FROM employees ";


        Mif::Db::IStatement::Parameters params;

        if (offset)
        {
            sql += "OFFSET $1::integer ";
            params.push_back(std::to_string(offset.Get()));
        }

        if (limit)
        {
            auto const index = !offset ? 1 : 2;
            sql += "LIMIT $" + std::to_string(index) + "::integer ";
            params.push_back(std::to_string(limit.Get()));
        }

        sql += ";";

        auto statement = connection->CreateStatement(sql);
        auto result = statement->Execute(params);

        Data::Response::Employees response;

        response.meta = CreateMeta();

        while (result->Read())
        {
            std::size_t index = 0;

            Data::Employee employee;

            employee.name = result->GetAsString(index++);
            employee.lastName = result->GetAsString(index++);
            employee.age = static_cast<std::uint32_t>(result->GetAsInt32(index++));
            employee.email = result->GetAsString(index++);
            employee.position = Mif::Reflection::FromString<Data::Position>(result->GetAsString(index++));
            employee.rate = result->GetAsDouble(index++);

            response.data.emplace(result->GetAsString(index), std::move(employee));
        }

        return response;
    }

}   // namespace Service
