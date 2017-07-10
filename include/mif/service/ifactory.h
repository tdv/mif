//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_IFACTORY_H__
#define __MIF_SERVICE_IFACTORY_H__

// STD
#include <string>

// MIF
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Service
    {

        struct IFactory
            : public Inherit<IService>
        {
            virtual IServicePtr Create(ServiceId id) = 0;
        };

        using IFactoryPtr = TServicePtr<IFactory>;

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_IFACTORY_H__
