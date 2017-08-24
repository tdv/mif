//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// MIF
#include "mif/application/iconfig.h"
#include "mif/net/http/web_service.h"
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
                }

            private:
                using ResultSerializer = Result<Mif::Net::Http::JsonSerializer>;
                template <typename T>
                using InputContent = Content<T, Mif::Net::Http::JsonContentParamConverter>;

                Mif::Service::IFactoryPtr m_factory;

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

                    //response.meta = CreateMeta();
                    response.data = storage->Create(employee.Get());

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
