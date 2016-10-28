//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_ISERVICE_FACTORY_H__
#define __MIF_SERVICE_ISERVICE_FACTORY_H__

// STD
#include <string>

// MIF
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Service
    {

        struct IServiceFactory
            : public IService
        {
            virtual IServicePtr Create(std::string const &id) = 0;
        };

        using IServiceFactoryPtr = std::shared_ptr<IServiceFactory>;

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_ISERVICE_FACTORY_H__
