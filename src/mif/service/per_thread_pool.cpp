//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     06.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

// MIF
#include "mif/common/log.h"
#include "mif/service/id/service.h"
#include "mif/service/creator.h"
#include "mif/service/icheckable.h"
#include "mif/service/ifactory.h"
#include "mif/service/ipool.h"

namespace Mif
{
    namespace Service
    {
        namespace Detail
        {
            class PerThreadPool
                : public Inherit<IPool>
            {
            public:
                PerThreadPool(Service::IFactoryPtr factory, Service::ServiceId serviceId)
                    : m_factory{factory}
                    , m_serviceId{serviceId}
                {
                    if (!m_factory)
                        throw std::invalid_argument{"[Mif::Service::Detail::PerThreadPool] The factory pointer must not be empty."};
                }

            private:
                using Services = std::map<std::thread::id, IServicePtr>;

                using LockType = std::mutex;
                using LockGuard = std::lock_guard<LockType>;

                Service::IFactoryPtr m_factory;
                Service::ServiceId m_serviceId;

                mutable LockType m_lock;
                mutable Services m_services;

                // IPool
                virtual IServicePtr GetService() const override final
                {
                    IServicePtr service;

                    {
                        auto id = std::this_thread::get_id();

                        {
                            LockGuard lock{m_lock};

                            auto iter = m_services.find(id);
                            if (iter != std::end(m_services))
                                service = iter->second;
                        }

                        if (service)
                        {
                            if (auto checkable = Service::Query<ICheckable>(service))
                            {
                                if (!checkable->IsGood())
                                {
                                    {
                                        LockGuard lock{m_lock};
                                        m_services.erase(id);
                                    }
                                    service.reset();
                                }
                            }
                        }

                        if (!service)
                        {
                            service = m_factory->Create(m_serviceId);

                            LockGuard lock{m_lock};
                            m_services.emplace(id, service);
                        }
                    }

                    return service;
                }
            };

        }   // namespace Detail
    }   // namespace Service
}   // namespace Mif


MIF_SERVICE_CREATOR
(
    Mif::Service::Id::PerThreadPool,
    Mif::Service::Detail::PerThreadPool,
    Mif::Service::IFactoryPtr,
    Mif::Service::ServiceId
)
