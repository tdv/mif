//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     05.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_IPOOL_H__
#define __MIF_SERVICE_IPOOL_H__

// MIF
#include "mif/service/iservice.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Service
    {

        struct IPool
            : public Inherit<IService>
        {
            virtual IServicePtr GetService() const = 0;

            template <typename T>
            TServicePtr<T> GetService() const
            {
                return Service::Cast<T>(GetService());
            }
        };

        using IPoolPtr = Service::TServicePtr<IPool>;

    }   // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_IPOOL_H__
