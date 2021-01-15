//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     05.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_ICHECKABLE_H__
#define __MIF_SERVICE_ICHECKABLE_H__

// MIF
#include "mif/service/iservice.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Service
    {

        struct ICheckable
            : public Inherit<IService>
        {
            virtual bool IsGood() const = 0;
        };

        using ICheckablePtr = Service::TServicePtr<ICheckable>;

    }   // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_ICHECKABLE_H__
