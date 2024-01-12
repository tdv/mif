//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_FACTORY_H__
#define __MIF_SERVICE_FACTORY_H__

// STD
#include <functional>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>

// MIF
#include "mif/common/crc32.h"
#include "mif/service/create.h"
#include "mif/service/detail/service.h"
#include "mif/service/ifactory.h"

namespace Mif
{
    namespace Service
    {

        class Factory
            : public Inherit<IFactory>
        {
        public:
            template <ServiceId Id, typename ... TArgs>
            void AddClass(TArgs && ... args)
            {
                auto creator = std::bind(::Mif::Service::Detail::Creators::Create<Id, TArgs ... >,
                        std::forward<TArgs>(args) ... );
                LockGuard lock(m_lock);
                m_creators.insert(std::make_pair(Id, std::move(creator)));
            }

            void AddInstance(ServiceId id, IServicePtr service)
            {
                LockGuard lock(m_lock);
                m_creators.insert(std::make_pair(id, [service] { return service; } ));
            }

            // IFactory
            virtual IServicePtr Create(ServiceId id) override final
            {
                Creator creator;
                {
                    LockGuard lock(m_lock);
                    auto iter = m_creators.find(id);
                    if (iter == std::end(m_creators))
                    {
                        throw std::runtime_error{"[Mif::Service::Factory::Create] Failed to create service "
                            "with id \"" + std::to_string(id) + "\". Creator not found."};
                    }
                    creator = iter->second;
                }
                return creator();
            }

        private:
            using LockType = std::mutex;
            using LockGuard = std::lock_guard<LockType>;
            
            LockType m_lock;

            using Creator = std::function<IServicePtr ()>;
            using Creators = std::map<ServiceId, Creator>;

            Creators m_creators;
        };

        using FactoryPtr = TServicePtr<Factory>;

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_FACTORY_H__
