//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_DETAIL_IOBJECT_MANAGER_PS_H__
#define __MIF_REMOTE_DETAIL_IOBJECT_MANAGER_PS_H__

// MIF
#include "mif/remote/ps.h"
#include "mif/remote/detail/iobject_manager.h"

namespace Mif
{
    namespace Service
    {
        namespace Meta
        {

            using ::Mif::Service::IService;

            MIF_REMOTE_PS_BEGIN(IService)
            MIF_REMOTE_PS_END()

        }   // namespace Meta
    }   // namespace Service
}   // namespace Mif

MIF_REMOTE_REGISTER_PS(Mif::Service::Meta::IService)

namespace Mif
{
    namespace Remote
    {
        namespace Detail
        {

            MIF_REMOTE_PS_BEGIN(IObjectManager)
                MIF_REMOTE_METHOD(CreateObject)
                MIF_REMOTE_METHOD(DestroyObject)
                MIF_REMOTE_METHOD(QueryInterface)
                MIF_REMOTE_METHOD(CloneReference)
            MIF_REMOTE_PS_END()

        }   // namespace Detail
    }   // namespace Remote
}   // namespace Mif

MIF_REMOTE_REGISTER_PS(Mif::Remote::Detail::IObjectManager)

#endif  // !__MIF_REMOTE_DETAIL_IOBJECT_MANAGER_PS_H__
