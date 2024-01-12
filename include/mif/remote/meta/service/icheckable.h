//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     05.2020
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_META_SERVICE_ICHECKABLE_H__
#define __MIF_REMOTE_META_SERVICE_ICHECKABLE_H__

// MIF
#include "mif/remote/ps.h"
#include "mif/service/icheckable.h"

namespace Mif
{
    namespace Service
    {
        namespace Meta
        {

            using Service::ICheckable;

            MIF_REMOTE_PS_BEGIN(ICheckable)
                MIF_REMOTE_METHOD(IsGood)
            MIF_REMOTE_PS_END()

        }   // namespace Meta
    }   // namespace Service
}   // namespace Mif

MIF_REMOTE_REGISTER_PS(Mif::Service::Meta::ICheckable)

#endif  // !__MIF_REMOTE_META_SERVICE_ICHECKABLE_H__
