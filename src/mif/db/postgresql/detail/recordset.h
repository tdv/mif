//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_DB_POSTGRESQL_DETAIL_RECORDSET_H__
#define __MIF_DB_POSTGRESQL_DETAIL_RECORDSET_H__

// STD
#include <list>
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
                    using Parameters = std::list<std::string/*value*/>;

                    Recordset(PGconn *connection, Service::IService *holder, std::string const &statementName,
                            Parameters const &parameters);

                private:
                    PGconn *m_connection;
                    Service::IServicePtr m_holder;
                    bool m_hasNext = true;
                    int m_currentRow = -1;
                    std::size_t m_fieldsCount = 0;

                    using ResultPtr = std::unique_ptr<PGresult, decltype(&PQclear)>;
                    ResultPtr m_result{nullptr, [] (PGresult *res) { if (res) PQclear(res); } };

                    void CheckIndex(std::size_t index) const;

                    // IRecordset
                    virtual bool Read() override final;
                    virtual std::size_t GetFieldsCount() const override final;
                    virtual bool IsNull(std::size_t index) const override final;
                    virtual std::string GetFieldName(std::size_t index) const override final;
                    virtual std::size_t GetFieldIndex(std::string const &name) const override final;
                    virtual std::string GetAsString(std::size_t index) const override final;
                    virtual std::int32_t GetAsInt32(std::size_t index) const override final;
                    virtual std::int64_t GetAsInt64(std::size_t index) const override final;
                    virtual double GetAsDouble(std::size_t index) const override final;
                };

            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Db
}   // namespace Mif

#endif  // !__MIF_DB_POSTGRESQL_DETAIL_RECORDSET_H__
