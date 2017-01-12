//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>

// MIF
#include "mif/common/unused.h"
#include "mif/service/creator.h"
#include "mif/service/ilocator.h"
#include "mif/service/id/service.h"

namespace Mif
{
    namespace Service
    {
        namespace Detail
        {
            namespace
            {

                class Locator
                    : public Inherit<ILocator>
                {
                private:
                    using LockType = std::mutex;
                    using LockGuard = std::lock_guard<LockType>;

                    mutable LockType m_lock;
                    std::map<ServiceId, IServicePtr> m_services;

                    // ILocator
                    virtual IServicePtr Get(ServiceId id) const override final
                    {
                        IServicePtr service;

                        {
                            LockGuard lock{m_lock};

                            auto iter = m_services.find(id);
                            if (iter == std::end(m_services))
                            {
                                throw std::invalid_argument{"[Mif::Service::Detail::Locator::Get] Failed to get service. "
                                    "Service with id \"" + std::to_string(id) + "\" not found."};
                            }

                            service = iter->second;
                        }

                        return service;
                    }

                    virtual bool Exists(ServiceId id) const override final
                    {
                        LockGuard lock{m_lock};
                        return m_services.find(id) != std::end(m_services);
                    }

                    virtual void Put(ServiceId id, IServicePtr service) override final
                    {
                        LockGuard lock{m_lock};
                        m_services[id] = service;
                    }

                    virtual void Remove(ServiceId id) override final
                    {
                        IServicePtr service;

                        {
                            LockGuard lock{m_lock};

                            auto iter = m_services.find(id);
                            if (iter == std::end(m_services))
                            {
                                throw std::invalid_argument{"[Mif::Service::Detail::Locator::Remove] Failed to remove service. "
                                    "Service with id \"" + std::to_string(id) + "\" not found."};
                            }

                            service = iter->second;

                            m_services.erase(iter);
                        }

                        Common::Unused(std::move(service));
                    }

                    virtual void Clear() override final
                    {
                        LockGuard lock{m_lock};
                        m_services.clear();
                    }
                };

            }   // namespace
        }   // namespace Detail
    }   // namespace Service
}   // namespace Mif

MIF_SERVICE_CREATOR
(
    ::Mif::Service::Id::Locator,
    ::Mif::Service::Detail::Locator
)
