//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_DB_IRECORDSET_H__
#define __MIF_DB_IRECORDSET_H__

// STD
#include <cstdint>
#include <string>

// MIF
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Db
    {

        struct IRecordset
            : public Service::Inherit<Service::IService>
        {
            virtual bool Read() = 0;
            virtual std::size_t GetFieldsCount() const = 0;
            virtual bool IsNull(std::size_t index) const = 0;
            virtual std::string GetFieldName(std::size_t index) const = 0;
            virtual std::string GetAsString(std::size_t index) const = 0;
            virtual std::int32_t GetAsInt32(std::size_t index) const = 0;
            virtual std::int64_t GetAsInt64(std::size_t index) const = 0;
            virtual double GetAsDouble(std::size_t index) const = 0;
        };

        using IRecordsetPtr = Service::TServicePtr<IRecordset>;

    }   // namespace Db
}   // namespace Mif

#endif  // !__MIF_DB_IRECORDSET_H__
