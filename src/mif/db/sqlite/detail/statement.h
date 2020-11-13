//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_DB_SQLITE_DETAIL_STATEMENT_H__
#define __MIF_DB_SQLITE_DETAIL_STATEMENT_H__

// STD
#include <memory>
#include <string>

// SQLITE
#include <sqlite3.h>

// MIF
#include "mif/db/istatement.h"
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Db
    {
        namespace SQLite
        {
            namespace Detail
            {

                class Statement
                    : public Service::Inherit<IStatement>
                {
                public:
                    Statement(sqlite3 *connection, Service::IService *holder, std::string const &query);

                private:
                    sqlite3 *m_connection;
                    Service::IServicePtr m_holder;

                    std::string m_query;

                    // IStatement
                    virtual IRecordsetPtr Execute(Parameters const &parameters) override final;
                };

            }   // namespace Detail
        }   // namespace SQLite
    }   // namespace Db
}   // namespace Mif

#endif  // !__MIF_DB_SQLITE_DETAIL_STATEMENT_H__
