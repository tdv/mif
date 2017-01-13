//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_DETAIL_SERVICE_H__
#define __MIF_SERVICE_DETAIL_SERVICE_H__

// STD
#include <atomic>
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
                : public IService
            {
            };

            template <typename T>
            class Service_Impl__ final
                : public std::enable_if<std::is_base_of<IService, T>::value || std::is_same<IService, T>::value, T>::type
                , public Detail::Fake_Service__
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
            };

        }   // namespace Detail
    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_DETAIL_SERVICE_H__
