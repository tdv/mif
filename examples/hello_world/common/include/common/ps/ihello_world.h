//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __HELLO_WORLD_COMMON_PS_IHELLO_WORLD_H__
#define __HELLO_WORLD_COMMON_PS_IHELLO_WORLD_H__

// STD
#include <string>

// MIF
#include <mif/remote/ps.h>

// THIS
#include "common/interface/ihello_world.h"

namespace Service
{
    namespace Meta
    {

        using namespace ::Service;

        MIF_REMOTE_PS_BEGIN(IHelloWorld)
            MIF_REMOTE_METHOD(AddWord)
            MIF_REMOTE_METHOD(GetText)
            MIF_REMOTE_METHOD(Clean)
        MIF_REMOTE_PS_END()

    }   // namespace Meta
}   // namespace Service

MIF_REMOTE_REGISTER_PS(Service::Meta::IHelloWorld)

#endif  // !__HELLO_WORLD_COMMON_PS_IHELLO_WORLD_H__
