//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_DB_POSTGRESQL_DETAIL_RECORDSET_H__
#define __MIF_DB_POSTGRESQL_DETAIL_RECORDSET_H__

// STD
#include <memory>
#include <string>

// LIBPR
#include <libpq-fe.h>

// MIF
#include "mif/db/irecordset.h"
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Db
    {
        namespace PostgreSql
        {
            namespace Detail
            {

                class Recordset
                    : public Service::Inherit<IRecordset>
                {
                public:
                    Recordset(PGconn *connection, Service::IService *holder, std::string const &statementName);

                    virtual ~Recordset();

                private:
                    PGconn *m_connection;
                    Service::IServicePtr m_holder;
                    std::string m_statementName;

                    using ResultPtr = std::unique_ptr<PGresult, decltype(&PQclear)>;
                    ResultPtr m_result{nullptr, [] (PGresult *res) { if (res) PQclear(res); } };

                    // IRecordset
                };

            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Db
}   // namespace Mif

#endif  // !__MIF_DB_POSTGRESQL_DETAIL_RECORDSET_H__
