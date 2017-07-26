//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __HTTP_CRUD_EMPLOYEE_SERVICE_H__
#define __HTTP_CRUD_EMPLOYEE_SERVICE_H__

// MIF
#include "mif/net/http/web_service.h"

// THIS
#include "data/data.h"

namespace Service
{

    class EmployeeService
        : public Mif::Net::Http::WebService
    {
    public:
        EmployeeService(std::string const &pathPrefix);

    private:
        using ResultSerializer = Result<Mif::Net::Http::JsonSerializer>;
        template <typename T>
        using InputContent = Content<T, Mif::Net::Http::JsonContentParamConverter>;

        Data::Response::Info CreateMeta(std::uint32_t code = 0, std::string const &message = {});

        // Web hadlers
        ResultSerializer Create(InputContent<Data::Employee> const &employee);
        ResultSerializer Read(Prm<Data::ID, Name("id")> const &id);
        ResultSerializer Update(Prm<Data::ID, Name("id")> const &id, InputContent<Data::Employee> const &employee);
        ResultSerializer Delete(Prm<Data::ID, Name("id")> const &id);

        ResultSerializer List(Prm<std::size_t, Name("offset")> const &offset,
                Prm<std::size_t, Name("limit")> const &limit);
    };

}   // namespace Service

#endif  // !__HTTP_CRUD_EMPLOYEE_SERVICE_H__
