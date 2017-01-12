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

            struct FakeService
                : public IService
            {
            };

            template <typename T>
            class Service final
                : public std::enable_if<std::is_base_of<IService, T>::value || std::is_same<IService, T>::value, T>::type
                , public Detail::FakeService
            {
            public:
                Service()
                {
                }

                template <typename ... TArgs>
                Service(TArgs && ... args)
                    : T{std::forward<TArgs>(args) ... }
                {
                }

            private:
                std::atomic_size_t m_counter;

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
