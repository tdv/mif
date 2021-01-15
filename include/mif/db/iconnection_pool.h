//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_DB_ICONNECTION_POOL_H__
#define __MIF_DB_ICONNECTION_POOL_H__

// MIF
#include "mif/db/iconnection.h"

namespace Mif
{
    namespace Db
    {

        struct IConnectionPool
            : public Service::Inherit<Service::IService>
        {
            virtual IConnectionPtr GetConnection() const = 0;
        };

        using IConnectionPoolPtr = Service::TServicePtr<IConnectionPool>;

    }   // namespace Db
}   // namespace Mif

#endif  // !__MIF_DB_ICONNECTION_POOL_H__
