//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_DB_SQLITE_DETAIL_RECORDSET_H__
#define __MIF_DB_SQLITE_DETAIL_RECORDSET_H__

// STD
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>

// SQLITE
#include <sqlite3.h>

// MIF
#include "mif/db/irecordset.h"
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Db
    {
        namespace SQLite
        {
            namespace Detail
            {

                class Recordset
                    : public Service::Inherit<IRecordset>
                {
                public:
                    using StatementPtr = std::unique_ptr<sqlite3_stmt, std::function<void (sqlite3_stmt *)>>;

                    Recordset(Service::IService *holder, StatementPtr statement);

                private:
                    Service::IServicePtr m_holder;
                    StatementPtr m_statement;
                    bool m_finished = false;
                    std::size_t m_fieldsCount = 0;
                    std::map<std::size_t, std::string> m_fieldsNames;
                    std::map<std::string, std::size_t> m_fieldsIndexes;

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
        }   // namespace SQLite
    }   // namespace Db
}   // namespace Mif

#endif  // !__MIF_DB_SQLITE_DETAIL_RECORDSET_H__
