//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// MIF
#include "mif/net/http/web_service.h"
#include <mif/service/creator.h>
#include <mif/service/ifactory.h>

// COMMON
#include "common/id/service.h"
#include "common/interface/iemployee_storage.h"

// THIS
#include "id/service.h"

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

            EmployeeService(std::string const &pathPrefix, Mif::Service::IFactoryPtr factory)
                : m_factory{std::move(factory)}
            {
                if (!m_factory)
                    throw std::invalid_argument{"[Service::Detail::EmployeeService] Empty factory ptr."};

                (void)pathPrefix;
                //AddHandler(pathPrefix + "/stat", this, &WebService::Stat);
            }

        private:
            Mif::Service::IFactoryPtr m_factory;

            // Web hadlers

        };

        }   // namespace
    }   // namespace Detail
}   // namespace Service

MIF_SERVICE_CREATOR
(
    ::Service::Id::Employee,
    ::Service::Detail::EmployeeService,
    std::string,
    Mif::Service::IFactoryPtr
)
