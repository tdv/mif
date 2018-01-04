//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __HTTP_CRUD_EMPLOYEE_SERVICE_H__
#define __HTTP_CRUD_EMPLOYEE_SERVICE_H__

// MIF
#include <mif/application/iconfig.h>
#include <mif/db/iconnection_pool.h>
#include <mif/net/http/converter/content/json.h>
#include <mif/net/http/web_service.h>
#include "mif/net/http/serializer/json.h"
#include <mif/service/ipool.h>

// THIS
#include "data/data.h"

namespace Service
{

    class EmployeeService
        : public Mif::Net::Http::WebService
    {
    public:
        EmployeeService(std::string const &pathPrefix, Mif::Application::IConfigPtr dbConfig);

    private:
        using ResultSerializer = Result<Mif::Net::Http::Serializer::Json>;
        template <typename T>
        using InputContent = Content<T, Mif::Net::Http::Converter::Content::Json>;

        Mif::Db::IConnectionPoolPtr m_connections;

        Data::Response::Info CreateMeta(std::uint32_t code = 0, std::string const &message = {}) const;
        std::string FormatExceptionMessage(std::string const &message) const;

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
