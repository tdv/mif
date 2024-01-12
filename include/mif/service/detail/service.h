//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_DETAIL_SERVICE_H__
#define __MIF_SERVICE_DETAIL_SERVICE_H__

// STD
#include <atomic>
#include <stdexcept>
#include <utility>

// MIF
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Service
    {
        namespace Detail
        {

            struct Fake_Service__
                : public Inherit<IService>
            {
            };

            template <typename T>
            class Service_Impl__ final
                : public std::enable_if<std::is_base_of<IService, T>::value || std::is_same<IService, T>::value, T>::type
                , public Fake_Service__
            {
            public:
                Service_Impl__()
                {
                }

                template <typename ... TArgs>
                Service_Impl__(TArgs && ... args)
                    : T{std::forward<TArgs>(args) ... }
                {
                }

                virtual ~Service_Impl__() = default;

            private:
                std::atomic_size_t m_counter{0};

                // IService
                virtual std::size_t AddRef() override final
                {
                    auto counter = ++m_counter;
                    return counter;
                }

                virtual std::size_t Release() override final
                {
                    auto counter = --m_counter;

                    if (!counter)
                        delete this;

                    return counter;
                }

                virtual bool Query(std::type_info const &typeInfo, void **service,
                        std::string const &serviceId, IService **holder) override final
                {
                    if (!service || *service)
                    {
                        throw std::invalid_argument{"[Mif::Service::Detail::Service_Impl__::Query] "
                                "Failed to query interface. Parameter \"service\" not empty. "
                                "Must be a pointer to pointer of the void type initialized by nullptr."};
                    }

                    if (typeInfo == typeid(IService))
                    {
                        *service = static_cast<Fake_Service__ *>(this);
                        return true;
                    }

                    if (T::QueryInterfaceInternal(service, static_cast<T *>(this), typeInfo, serviceId))
                        return true;

                    if (auto *proxy = dynamic_cast<IProxyBase_Mif_Remote_ *>(this))
                        return proxy->_Mif_Remote_QueryRemoteInterface(service, typeInfo, serviceId, holder);

                    return false;
                }
            };

        }   // namespace Detail
    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_DETAIL_SERVICE_H__
