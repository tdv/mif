//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/common/log.h>
#include <mif/service/creator.h>

// COMMON
#include "common/id/service.h"
#include "common/interface/iface.h"

namespace Service
{
    namespace Detail
    {
        namespace
        {

            class Service
                : public Mif::Service::Inherit<IEmployee>
            {
            public:
                Service()
                {
                    MIF_LOG(Info) << "Service";
                }

                ~Service()
                {
                    MIF_LOG(Info) << "~Service";
                }

            private:
                // IHuman
                virtual std::string GetName() const override final
                {
                    MIF_LOG(Info) << "GetName";
                    return "Ivan";
                }

                virtual std::size_t GetAge() const override final
                {
                    MIF_LOG(Info) << "GetAge";
                    return 33;
                }

                // IDeveloper
                virtual void Development() override final
                {
                    MIF_LOG(Info) << "Development";
                }

                virtual void BugFixes() override final
                {
                    MIF_LOG(Info) << "BugFixes";
                }

                // IManager
                virtual void Planning() override final
                {
                    MIF_LOG(Info)  << "Planning";
                }

                virtual void Monitoring() override final
                {
                    MIF_LOG(Info) << "Monitoring";
                }

                // IEmployee
                virtual double GetRate() override final
                {
                    MIF_LOG(Info) << "GetRate";
                    return 100000.00;
                }
            };

        }   // namespace
    }   // namespace Detail
}   // namespace Service

MIF_SERVICE_CREATOR
(
    ::Service::Id::Service,
    ::Service::Detail::Service
)
