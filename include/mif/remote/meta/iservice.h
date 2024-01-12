//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_META_ISERVICE_H__
#define __MIF_REMOTE_META_ISERVICE_H__

// MIF
#include "mif/remote/ps.h"
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Service
    {
        namespace Meta
        {

            using Service::IService;

            MIF_REMOTE_PS_BEGIN(IService)
            MIF_REMOTE_PS_END()

        }   // namespace Meta
    }   // namespace Service
}   // namespace Mif

MIF_REMOTE_REGISTER_PS(Mif::Service::Meta::IService)

#endif  // !__MIF_REMOTE_META_ISERVICE_H__
