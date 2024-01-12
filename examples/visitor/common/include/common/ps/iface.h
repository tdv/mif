//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __VISITOR_COMMON_PS_IFACE_H__
#define __VISITOR_COMMON_PS_IFACE_H__

// MIF
#include <mif/remote/ps.h>

// THIS
#include "common/interface/iface.h"

namespace Service
{
    namespace Meta
    {

        using namespace ::Service;

        MIF_REMOTE_PS_BEGIN(IMessage)
            MIF_REMOTE_METHOD(GetText)
        MIF_REMOTE_PS_END()

        MIF_REMOTE_PS_BEGIN(IMessageVisitor)
            MIF_REMOTE_METHOD(Visit)
        MIF_REMOTE_PS_END()

        MIF_REMOTE_PS_BEGIN(IViewer)
            MIF_REMOTE_METHOD(Accept)
        MIF_REMOTE_PS_END()

    }   // namespace Meta
}   // namespace Service

MIF_REMOTE_REGISTER_PS(Service::Meta::IMessage)
MIF_REMOTE_REGISTER_PS(Service::Meta::IMessageVisitor)
MIF_REMOTE_REGISTER_PS(Service::Meta::IViewer)

#endif  // !__VISITOR_COMMON_PS_IFACE_H__
