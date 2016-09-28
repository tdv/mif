#ifndef __HELLO_WORLD_COMMON_PS_IFACE_H__
#define __HELLO_WORLD_COMMON_PS_IFACE_H__

// STD
#include <string>

// MIF
#include <mif/remote/ps.h>

// THIS
#include "common/interface/iface.h"

MIF_REMOTE_PS_BEGIN(IFace)
    MIF_REMOTE_METHOD(SayHello)
MIF_REMOTE_PS_END()

#endif  // !__HELLO_WORLD_COMMON_PS_IFACE_H__
