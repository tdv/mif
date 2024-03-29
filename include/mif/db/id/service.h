//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_DB_ID_SERVICE_H__
#define __MIF_DB_ID_SERVICE_H__

// MIF
#include "mif/common/config.h"
#include "mif/common/crc32.h"

namespace Mif
{
    namespace Db
    {
        namespace Id
        {
            namespace Service
            {

                enum
                {
#ifdef MIF_WITH_POSTGRESQL
                    PostgreSQL = Common::Crc32("Mif.Db.Service.Connection.PostgreSQL"),
                    PostgresConnectionPool = Common::Crc32("Mif.Db.Service.PostgresConnectionPool")
#endif  // !MIF_WITH_POSTGRESQL
                };

            }   // namespace Service
        }   // namespace Id
    }   // namespace Db
}   // namespace Mif

#endif  // !__MIF_DB_ID_SERVICE_H__
