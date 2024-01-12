//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_ILOCATOR_H__
#define __MIF_SERVICE_ILOCATOR_H__

// MIF
#include "mif/service/create.h"
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Service
    {

        struct ILocator
            : public Inherit<IService>
        {
            MIF_SERVICE_ID("Mif.Service.ILocator")

            virtual IServicePtr Get(ServiceId id) const = 0;
            virtual bool Exists(ServiceId id) const = 0;
            virtual void Put(ServiceId id, IServicePtr service) = 0;
            virtual void Remove(ServiceId id) = 0;
            virtual void Clear() = 0;

            template <typename T>
            TServicePtr<T> Get(ServiceId id) const
            {
                return ::Mif::Service::Cast<T>(Get(id));
            }

            template <ServiceId Id, typename ... TArgs>
            void Put(TArgs && ... args)
            {
                Put(Id, Create<Id>(std::forward<TArgs>(args) ... ));
            }
        };

        using ILocatorPtr = TServicePtr<ILocator>;

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_ILOCATOR_H__
