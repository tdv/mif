//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_DB_ICONNECTION_H__
#define __MIF_DB_ICONNECTION_H__

// MIF
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Db
    {

        struct IConnection
            : public Service::Inherit<Service::IService>
        {
        };

        using IConnectionPtr = Service::TServicePtr<IConnection>;

    }   // namespace Db
}   // namespace Mif

#endif  // !__MIF_DB_ICONNECTION_H__
