#ifndef __MIF_NET_ISUBSCRIBER_H__
#define __MIF_NET_ISUBSCRIBER_H__

// MIF
#include "mif/common/types.h"

namespace Mif
{
    namespace Net
    {

        struct ISubscriber
        {
            virtual ~ISubscriber() = default;
            virtual void OnData(Common::Buffer buffer) = 0;
        };

    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_ISUBSCRIBER_H__
