//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// THIS
#include "employee_service.h"
#include "data/meta/data.h"

namespace Service
{

    EmployeeService::EmployeeService(std::string const &pathPrefix)
    {
        AddHandler(pathPrefix + "/create", this, &EmployeeService::Create);
        AddHandler(pathPrefix + "/read", this, &EmployeeService::Read);
        AddHandler(pathPrefix + "/update", this, &EmployeeService::Update);
        AddHandler(pathPrefix + "/delete", this, &EmployeeService::Delete);

        AddHandler(pathPrefix + "/list", this, &EmployeeService::List);
    }

    Data::Response::Info EmployeeService::CreateMeta(std::uint32_t code, std::string const &message)
    {
        Data::Response::Info meta;
        // TODO: fill node and timestamp
        meta.status.code = code;
        meta.status.message = message;
        return meta;
    }

    EmployeeService::ResultSerializer
    EmployeeService::Create(InputContent<Data::Employee> const &employee)
    {
        if (!employee)
            throw std::invalid_argument{"No data."};

        Data::Response::Id response;

        response.meta = CreateMeta();
        // TODO: fill data
        //response.data =

        return response;
    }

    EmployeeService::ResultSerializer
    EmployeeService::Read(Prm<Data::ID, Name("id")> const &id)
    {
        if (!id || id.Get().empty())
            throw std::invalid_argument{"Id must not be empty."};

        Data::Response::Employee response;

        response.meta = CreateMeta();
        // TODO: fill data
        //response.data =

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

        Data::Response::Void response;
        response.meta = CreateMeta();
        return response;
    }

    EmployeeService::ResultSerializer
    EmployeeService::Delete(Prm<Data::ID, Name("id")> const &id)
    {
        if (!id || id.Get().empty())
            throw std::invalid_argument{"Id must not be empty."};

        Data::Response::Void response;
        response.meta = CreateMeta();
        return response;
    }

    EmployeeService::ResultSerializer
    EmployeeService::List(Prm<std::size_t, Name("offset")> const &offset,
                          Prm<std::size_t, Name("limit")> const &limit)
    {
        (void)offset;
        (void)limit;

        Data::Response::Employees response;

        response.meta = CreateMeta();
        // TODO: fill data
        //response.data =

        return response;
    }

}   // namespace Service
