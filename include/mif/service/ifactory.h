//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_IFACTORY_H__
#define __MIF_SERVICE_IFACTORY_H__

// STD
#include <string>

// MIF
#include "mif/common/crc32.h"
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Service
    {

        struct IFactory
            : public Inherit<IService>
        {
            virtual IServicePtr Create(ServiceId id) = 0;

            template <typename T>
            TServicePtr<T> Create(ServiceId id)
            {
                return Service::Cast<T>(Create(id));
            }

            template <typename T>
            TServicePtr<T> Create(std::string const &id)
            {
                return Create<T>(Common::Crc32str(id));
            }

        };

        using IFactoryPtr = TServicePtr<IFactory>;

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_IFACTORY_H__
