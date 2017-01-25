//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __PROTO_STACK_COMMON_PS_IFACE_H__
#define __PROTO_STACK_COMMON_PS_IFACE_H__

// MIF
#include <mif/remote/ps.h>

// THIS
#include "common/interface/iface.h"

namespace Service
{
    namespace Meta
    {

        using namespace ::Service;

        MIF_REMOTE_PS_BEGIN(IFace)
            MIF_REMOTE_METHOD(SayHello)
        MIF_REMOTE_PS_END()

    }   // namespace Meta
}   // namespace Service

MIF_REMOTE_REGISTER_PS(Service::Meta::IFace)

#endif  // !__PROTO_STACK_COMMON_PS_IFACE_H__
