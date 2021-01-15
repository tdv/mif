//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MICROSERVICES_COMMON_INTERFACE_PS_IEMPPLOYEE_STORAGE_H__
#define __MICROSERVICES_COMMON_INTERFACE_PS_IEMPPLOYEE_STORAGE_H__

// MIF
#include <mif/remote/ps.h>

// THIS
#include "common/interface/iemployee_storage.h"
#include "common/data/meta/data.h"

namespace Common
{
    namespace Meta
    {

        using namespace ::Common;

        MIF_REMOTE_PS_BEGIN(IEmployeeStorage)
            MIF_REMOTE_METHOD(Create)
            MIF_REMOTE_METHOD(Read)
            MIF_REMOTE_METHOD(Update)
            MIF_REMOTE_METHOD(Delete)
            MIF_REMOTE_METHOD(List)
        MIF_REMOTE_PS_END()

    }   // namespace Meta
}   // namespace Common

MIF_REMOTE_REGISTER_PS(Common::Meta::IEmployeeStorage)

#endif  // !__MICROSERVICES_COMMON_INTERFACE_PS_IEMPPLOYEE_STORAGE_H__
