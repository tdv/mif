//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __INHERITANCE_COMMON_PS_IFACE_H__
#define __INHERITANCE_COMMON_PS_IFACE_H__

// MIF
#include <mif/remote/ps.h>

// THIS
#include "common/interface/iface.h"

namespace Service
{
    namespace Meta
    {

        using namespace ::Service;

        MIF_REMOTE_PS_BEGIN(IHuman)
            MIF_REMOTE_METHOD(GetName)
            MIF_REMOTE_METHOD(GetAge)
        MIF_REMOTE_PS_END()

        MIF_REMOTE_PS_BEGIN(IDeveloper)
            MIF_REMOTE_METHOD(Development)
            MIF_REMOTE_METHOD(BugFixes)
        MIF_REMOTE_PS_END()

        MIF_REMOTE_PS_BEGIN(IManager)
            MIF_REMOTE_METHOD(Planning)
            MIF_REMOTE_METHOD(Monitoring)
        MIF_REMOTE_PS_END()

        MIF_REMOTE_PS_BEGIN(IEmployee)
            MIF_REMOTE_METHOD(GetWages)
        MIF_REMOTE_PS_END()

    }   // namespace Meta
}   // namespace Service

MIF_REMOTE_REGISTER_PS(Service::Meta::IHuman)
MIF_REMOTE_REGISTER_PS(Service::Meta::IDeveloper)
MIF_REMOTE_REGISTER_PS(Service::Meta::IManager)
MIF_REMOTE_REGISTER_PS(Service::Meta::IEmployee)

#endif  // !__INHERITANCE_COMMON_PS_IFACE_H__
