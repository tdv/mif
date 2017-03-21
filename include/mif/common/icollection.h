//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_ICOLLECTION_H__
#define __MIF_COMMON_ICOLLECTION_H__

// MIF
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Common
    {

        struct ICollection
            : public Service::Inherit<Service::IService>
        {
            virtual bool HasNext() const = 0;
            virtual bool IsEmpty() const = 0;
            virtual Service::IServicePtr First() = 0;
            virtual Service::IServicePtr Next() = 0;
        };

        using ICollectionPtr = Service::TServicePtr<ICollection>;

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_ICOLLECTION_H__
