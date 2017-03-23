//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include "mif/db/iconnection.h"
#include "mif/db/id/service.h"
#include "mif/service/creator.h"

namespace Mif
{
    namespace Db
    {
        namespace Detail
        {

            class Connection
                : public Service::Inherit<IConnection>
            {
            public:
                Connection()
                {
                }

            private:
                // IConnection
            };

        }   // namespace Detail
    }   // namespace Db
}   // namespace Mif

MIF_SERVICE_CREATOR
(
    Mif::Db::Id::Service::SQLite,
    Mif::Db::Detail::Connection
)
