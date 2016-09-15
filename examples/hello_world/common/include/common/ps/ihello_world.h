#ifndef __HELLO_WORLD_COMMON_PS_IHELLO_WORLD_H__
#define __HELLO_WORLD_COMMON_PS_IHELLO_WORLD_H__

// STD
#include <string>

// MIF
#include <mif/remote/ps.h>

// THIS
#include "common/interface/ihello_world.h"

MIF_REMOTE_PS_BEGIN(IHelloWorld)
    MIF_REMOTE_METHOD(AddWord)
    MIF_REMOTE_METHOD(GetText)
    MIF_REMOTE_METHOD(Clean)
MIF_REMOTE_PS_END()

#endif  // !__HELLO_WORLD_COMMON_PS_IHELLO_WORLD_H__
