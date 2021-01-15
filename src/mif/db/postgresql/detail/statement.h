//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_DB_POSTGRESQL_DETAIL_STATEMENT_H__
#define __MIF_DB_POSTGRESQL_DETAIL_STATEMENT_H__

// STD
#include <memory>
#include <string>

// LIBPR
#include <libpq-fe.h>

// MIF
#include "mif/db/istatement.h"
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Db
    {
        namespace PostgreSql
        {
            namespace Detail
            {

                class Statement
                    : public Service::Inherit<IStatement>
                {
                public:
                    using ResultPtr = std::unique_ptr<PGresult, decltype(&PQclear)>;

                    Statement(PGconn *connection, Service::IService *holder, std::string const &query);

                    virtual ~Statement();

                private:
                    PGconn *m_connection;
                    Service::IServicePtr m_holder;
                    std::string m_name;

                    // IStatement
                    virtual IRecordsetPtr Execute(Parameters const &parameters) override final;
                };

            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Db
}   // namespace Mif

#endif  // !__MIF_DB_POSTGRESQL_DETAIL_STATEMENT_H__
