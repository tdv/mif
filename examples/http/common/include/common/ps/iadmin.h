//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __HTTP_COMMON_PS_IADMIN_H__
#define __HTTP_COMMON_PS_IADMIN_H__

// STD
#include <string>

// MIF
#include <mif/remote/ps.h>

// THIS
#include "common/interface/iadmin.h"

namespace Service
{
    namespace Meta
    {

        using namespace ::Service;

        MIF_REMOTE_PS_BEGIN(IAdmin)
            MIF_REMOTE_METHOD(SetTitle)
            MIF_REMOTE_METHOD(SetBody)
            MIF_REMOTE_METHOD(GetPage)
        MIF_REMOTE_PS_END()

    }   // namespace Meta
}   // namespace Service

MIF_REMOTE_REGISTER_PS(Service::Meta::IAdmin)

#endif  // !__HTTP_COMMON_PS_IADMIN_H__
