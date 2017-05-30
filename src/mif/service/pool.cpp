//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     05.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>

// MIF
#include "mif/common/log.h"
#include "mif/service/id/service.h"
#include "mif/service/creator.h"
#include "mif/service/make.h"
#include "mif/service/icheckable.h"
#include "mif/service/ifactory.h"
#include "mif/service/ipool.h"

namespace Mif
{
    namespace Service
    {
        namespace Detail
        {
            class Pool
                : public Inherit<IPool>
            {
            public:
                Pool(std::uint32_t limit, Service::IFactoryPtr factory, Service::ServiceId serviceId)
                    : m_limit{limit}
                    , m_factory{factory}
                    , m_serviceId{serviceId}
                {
                    if (!m_limit)
                        throw std::invalid_argument{"[Mif::Service::Detail::Pool] The limit must be greater than 0."};
                    if (!m_factory)
                        throw std::invalid_argument{"[Mif::Service::Detail::Pool] The factory pointer must not be empty."};
                }

            private:
                using Services = std::map<IServicePtr, bool /*is busy*/>;

                using LockType = std::mutex;
                using LockGuard = std::lock_guard<LockType>;

                std::uint32_t m_limit;
                Service::IFactoryPtr m_factory;
                Service::ServiceId m_serviceId;

                mutable LockType m_lock;
                mutable Services m_services;

                // IPool
                virtual ProxyPtr GetService() const override final
                {
                    ProxyPtr proxy;

                    {
                        LockGuard lock{m_lock};

                        for (auto i = std::begin(m_services) ; i != std::end(m_services) ; )
                        {
                            if (auto checkable = Service::Query<ICheckable>(i->first))
                            {
                                if (!checkable->IsGood())
                                {
                                    m_services.erase(i++);
                                    continue;
                                }
                            }

                            if (!i->second)
                            {
                                auto holder = Make<Holder>(const_cast<Pool *>(this), i->first);
                                proxy = Make<Proxy<IService>, Proxy<IService>>(holder, i->first);

                                break;
                            }

                            ++i;
                        }

                        if (!proxy)
                        {
                            if (m_services.size() >= m_limit)
                            {
                                throw std::runtime_error{"[Mif::Service::Detail::Pool::GetService] Failed to get service. "
                                    "Maximum limit reached (" + std::to_string(m_limit) + ")."};
                            }

                            auto newInst = m_factory->Create(m_serviceId);
                            m_services.emplace(newInst, false);
                            auto holder = Make<Holder>(const_cast<Pool *>(this), newInst);
                            proxy = Make<Proxy<IService>, Proxy<IService>>(holder, newInst);
                        }
                    }

                    return proxy;
                }

                class Holder
                    : public Inherit<IService>
                {
                public:
                    Holder(Pool *pool, IServicePtr service)
                        : m_pool{pool}
                        , m_service{service}
                    {
                        auto iter = m_pool->m_services.find(m_service);
                        if (iter == std::end(m_pool->m_services))
                            throw std::logic_error{"Service for lock not found."};
                        iter->second = true;
                    }

                    ~Holder()
                    {
                        try
                        {
                            LockGuard lock{m_pool->m_lock};
                            auto iter = m_pool->m_services.find(m_service);
                            if (iter == std::end(m_pool->m_services))
                                throw std::logic_error{"Service for unlock not found."};
                            iter->second = false;
                        }
                        catch (std::exception const &e)
                        {
                            MIF_LOG(Warning) << "[Mif::Service::Detail::Pool::~Holder] Error: " << e.what();
                        }
                    }

                private:
                    TIntrusivePtr<Pool> m_pool;
                    IServicePtr m_service;
                };

                friend class Holder;
            };

        }   // namespace Detail
    }   // namespace Service
}   // namespace Mif


MIF_SERVICE_CREATOR
(
    Mif::Service::Id::Pool,
    Mif::Service::Detail::Pool,
    std::uint32_t,
    Mif::Service::IFactoryPtr,
    Mif::Service::ServiceId
)
