#ifndef __MIF_SERVICE_SERVICE_FACTORY_H__
#define __MIF_SERVICE_SERVICE_FACTORY_H__

// STD
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>

// MIF
#include "mif/service/iservice_factory.h"

namespace Mif
{
    namespace Service
    {

        class ServiceFactory final
            : public IServiceFactory
        {
        public:
            template <typename T>
            void AddClass(std::string const &id)
            {
                auto factory = std::make_shared<Factory<T>>();
                {
                    LockGuard lock(m_lock);
                    m_services.insert(std::make_pair(id, std::move(factory)));
                }
            }

            // IServiceFactory
            virtual IServicePtr Create(std::string const &id) override final
            {
                IServiceFactoryPtr factory;
                {
                    LockGuard lock(m_lock);
                    auto iter = m_services.find(id);
                    if (iter == std::end(m_services))
                    {
                        throw std::runtime_error{"[Mif::Service::ServiceFactory::Create] Failed to create service "
                            "with id \"" + id + "\". Creator not found."};
                    }
                    factory = iter->second;
                }
                return factory->Create(id);
            }

        private:
            using LockType = std::mutex;
            using LockGuard = std::lock_guard<LockType>;
            using Services = std::map<std::string/*id*/, IServiceFactoryPtr>;

            LockType m_lock;
            Services m_services;

            template <typename T>
            class Factory final
                : public IServiceFactory
            {
            private:
                virtual IServicePtr Create(std::string const &id) override final
                {
                    auto instance = std::make_shared<T>();
                    auto sevice = std::dynamic_pointer_cast<IService>(instance);
                    if (!sevice)
                        throw std::runtime_error{"Failed to create service with id \"" + id + "\". Bad cast to IService interface."};
                    return sevice;
                }
            };
        };

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_SERVICE_FACTORY_H__
