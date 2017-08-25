//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// BOOST
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// MIF
#include <mif/application/iconfig.h>
#include <mif/common/log.h>
#include <mif/net/http/web_service.h>
#include <mif/remote/predefined/utility.h>
#include <mif/service/creator.h>
#include <mif/service/ifactory.h>

// COMMON
#include "common/id/service.h"
#include "common/interface/iemployee_storage.h"
#include "common/interface/ps/iemployee_storage.h"

// THIS
#include "id/service.h"
#include "data/meta/responses.h"

namespace Service
{
    namespace Detail
    {
        namespace
        {

            class EmployeeService
                : public Mif::Service::Inherit<Mif::Net::Http::WebService>
            {
            public:

                EmployeeService(std::string const &pathPrefix, Mif::Application::IConfigPtr config)
                {
                    if (!config)
                        throw std::invalid_argument{"[Service::Detail::EmployeeService] No config."};

                    auto const host = config->GetValue("host");
                    auto const port = config->GetValue("port");
                    auto const workers = config->GetValue<std::uint16_t>("workers");
                    std::chrono::microseconds const timeout{config->GetValue<std::uint32_t>("timeout")};

                    m_factory = Mif::Remote::Predefined::CreateTcpClientServiceFactory(
                            host, port, workers, timeout
                        );

                    AddHandler(pathPrefix + "/create", this, &EmployeeService::Create);
                    AddHandler(pathPrefix + "/read", this, &EmployeeService::Read);
                    AddHandler(pathPrefix + "/update", this, &EmployeeService::Update);
                    AddHandler(pathPrefix + "/delete", this, &EmployeeService::Delete);

                    AddHandler(pathPrefix + "/list", this, &EmployeeService::List);
                }

            private:
                using ResultSerializer = Result<Mif::Net::Http::JsonSerializer>;
                template <typename T>
                using InputContent = Content<T, Mif::Net::Http::JsonContentParamConverter>;

                Mif::Service::IFactoryPtr m_factory;

                Data::Response::Info CreateMeta(std::uint32_t code = 0, std::string const &message = {}) const
                {
                    Data::Response::Info meta;
                    meta.timestamp = boost::posix_time::to_iso_string(boost::posix_time::second_clock::universal_time());
                    meta.node = boost::asio::ip::host_name();
                    meta.status.code = code;
                    meta.status.message = message;
                    return meta;
                }

                std::string FormatExceptionMessage(std::string const &message) const
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

                // Web hadlers
                EmployeeService::ResultSerializer
                Create(InputContent<Common::Data::Employee> const &employee)
                {
                    if (!employee)
                        throw std::invalid_argument{"No data."};

                    auto storage = m_factory->Create<Common::IEmployeeStorage>(
                            Common::Service::Id::PGEmployeeStorage
                        );

                    Data::Response::Id response;

                    response.meta = CreateMeta();
                    response.data = storage->Create(employee.Get());

                    return response;
                }

                EmployeeService::ResultSerializer
                Read(Prm<Common::Data::ID, Name("id")> const &id)
                {
                    if (!id || id.Get().empty())
                        throw std::invalid_argument{"Id must not be empty."};

                    auto storage = m_factory->Create<Common::IEmployeeStorage>(
                            Common::Service::Id::PGEmployeeStorage
                        );

                    Data::Response::Employee response;

                    response.meta = CreateMeta();
                    response.data = storage->Read(id.Get());

                    return response;
                }

                EmployeeService::ResultSerializer
                Update(Prm<Common::Data::ID, Name("id")> const &id,
                        InputContent<Common::Data::Employee> const &employee)
                {
                    if (!id || id.Get().empty())
                        throw std::invalid_argument{"Id must not be empty."};

                    if (!employee)
                        throw std::invalid_argument{"No data."};

                    auto storage = m_factory->Create<Common::IEmployeeStorage>(
                            Common::Service::Id::PGEmployeeStorage
                        );

                    storage->Update(id.Get(), employee.Get());

                    Data::Response::Void response;

                    response.meta = CreateMeta();

                    return response;
                }

                EmployeeService::ResultSerializer
                Delete(Prm<Common::Data::ID, Name("id")> const &id)
                {
                    if (!id || id.Get().empty())
                        throw std::invalid_argument{"Id must not be empty."};

                    auto storage = m_factory->Create<Common::IEmployeeStorage>(
                            Common::Service::Id::PGEmployeeStorage
                        );

                    storage->Delete(id.Get());

                    Data::Response::Void response;

                    response.meta = CreateMeta();

                    return response;
                }

                EmployeeService::ResultSerializer
                List(Prm<std::size_t, Name("offset")> const &offset,
                        Prm<std::size_t, Name("limit")> const &limit)
                {
                    auto storage = m_factory->Create<Common::IEmployeeStorage>(
                            Common::Service::Id::PGEmployeeStorage
                        );

                    Data::Response::Employees response;

                    response.meta = CreateMeta();
                    response.data = storage->List(offset ? offset.Get() : std::numeric_limits<std::size_t>::max(),
                            limit ? limit.Get() : std::numeric_limits<std::size_t>::max());

                    return response;
                }

            };

        }   // namespace
    }   // namespace Detail
}   // namespace Service

MIF_SERVICE_CREATOR
(
    ::Service::Id::Employee,
    ::Service::Detail::EmployeeService,
    std::string,
    Mif::Application::IConfigPtr
)
