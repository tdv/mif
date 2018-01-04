//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_ROOT_LOCATOR_H__
#define __MIF_SERVICE_ROOT_LOCATOR_H__

// MIF
#include "mif/service/create.h"
#include "mif/service/ilocator.h"
#include "mif/service/id/service.h"

namespace Mif
{
    namespace Service
    {
        namespace Detail
        {

            template <typename T>
            class RootLocator final
            {
            public:
                static ILocatorPtr Get()
                {
                    return m_instance.m_locator;
                }

            private:
                static RootLocator m_instance;
                ILocatorPtr m_locator{Create<Id::Locator, ILocator>()};

                RootLocator() = default;

                RootLocator(RootLocator const &) = delete;
                RootLocator& operator = (RootLocator const &) = delete;
                RootLocator(RootLocator &&) = delete;
                RootLocator& operator = (RootLocator &&) = delete;
            };

            template <typename T>
            RootLocator<T> RootLocator<T>::m_instance;

        }   // namespace Detail

        using RootLocator = Detail::RootLocator<struct RootLocatorTag>;

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_ROOT_LOCATOR_H__
