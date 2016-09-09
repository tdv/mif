#ifndef __MIF_REMOTE_DETAIL_IOBJECT_MANAGER_PS_H__
#define __MIF_REMOTE_DETAIL_IOBJECT_MANAGER_PS_H__

// MIF
#include "mif/remote/ps.h"
#include "mif/remote/detail/iobject_manager.h"

namespace Mif
{
    namespace Remote
    {
        namespace Detail
        {

            MIF_REMOTE_PS_BEGIN(IObjectManager)
                MIF_REMOTE_METHOD(CreateObject)
                MIF_REMOTE_METHOD(DestroyObject)
            MIF_REMOTE_PS_END()

        }   // namespace Detail
    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_DETAIL_IOBJECT_MANAGER_PS_H__
