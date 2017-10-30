//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_DB_ID_SERVICE_H__
#define __MIF_DB_ID_SERVICE_H__

// MIF
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
                    PostgreSQL = Common::Crc32("Mif.Db.Service.Connection.PostgreSQL"),
                    SQLite = Common::Crc32("Mif.Db.Service.Connection.SQLite"),

                    PostgresPerThreadPool = Common::Crc32("Mif.Db.Service.PostgresPerThreadPool")
                };

            }   // namespace Service
        }   // namespace Id
    }   // namespace Db
}   // namespace Mif

#endif  // !__MIF_DB_ID_SERVICE_H__
