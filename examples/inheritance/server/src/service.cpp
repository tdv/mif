//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>
#include <mutex>

// MIF
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
                : public IEmployee
            {
            public:
                Service()
                {
                    LockGuard lock(m_lock);
                    std::cout << "Service" << std::endl;
                }

                ~Service()
                {
                    LockGuard lock(m_lock);
                    std::cout << "~Service" << std::endl;
                }

            private:
                using LockType = std::mutex;
                using LockGuard = std::lock_guard<LockType>;

                mutable LockType m_lock;

                // IHuman
                virtual std::string GetName() const override final
                {
                    {
                        LockGuard lock(m_lock);
                        std::cout << "GetName" << std::endl;
                    }
                    return "Ivan";
                }

                virtual std::size_t GetAge() const override final
                {
                    {
                        LockGuard lock(m_lock);
                        std::cout << "GetAge" << std::endl;
                    }
                    return 33;
                }

                // IDeveloper
                virtual void Development() override final
                {
                    LockGuard lock(m_lock);
                    std::cout << "Development" << std::endl;
                }

                virtual void BugFixes() override final
                {
                    LockGuard lock(m_lock);
                    std::cout << "BugFixes" << std::endl;
                }

                // IManager
                virtual void Planning() override final
                {
                    LockGuard lock(m_lock);
                    std::cout << "Planning" << std::endl;
                }

                virtual void Monitoring() override final
                {
                    LockGuard lock(m_lock);
                    std::cout << "Monitoring" << std::endl;
                }

                // IEmployee
                virtual double GetWages() override final
                {
                    {
                        LockGuard lock(m_lock);
                        std::cout << "GetWages" << std::endl;
                    }
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
