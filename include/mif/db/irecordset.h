//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_DB_IRECORDSET_H__
#define __MIF_DB_IRECORDSET_H__

// MIF
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Db
    {

        struct IRecordset
            : public Service::Inherit<Service::IService>
        {
        };

        using IRecordsetPtr = Service::TServicePtr<IRecordset>;

    }   // namespace Db
}   // namespace Mif

#endif  // !__MIF_DB_IRECORDSET_H__
