//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_FACTORY_H__
#define __MIF_SERVICE_FACTORY_H__

// STD
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>

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
            : public IFactory
        {
        public:
            template <ServiceId Id>
            void AddClass()
            {
                LockGuard lock(m_lock);
                m_creators.insert(std::make_pair(Id, ::Mif::Service::Create<Id>));
            }

            // IFactory
            virtual IServicePtr Create(ServiceId id) override final
            {
                Creator creator = nullptr;
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

            virtual IServicePtr Create(std::string const &id) override final
            {
                Creator creator = nullptr;
                {
                    LockGuard lock(m_lock);
                    auto iter = m_creators.find(Common::Crc32str(id));
                    if (iter == std::end(m_creators))
                    {
                        throw std::runtime_error{"[Mif::Service::Factory::Create] Failed to create service "
                            "with id \"" + id + "\". Creator not found."};
                    }
                    creator = iter->second;
                }
                return creator();
            }

        private:
            using LockType = std::mutex;
            using LockGuard = std::lock_guard<LockType>;
            using Creator = IServicePtr (*)();
            using Creators = std::map<ServiceId, Creator>;

            LockType m_lock;
            Creators m_creators;
        };

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_FACTORY_H__
