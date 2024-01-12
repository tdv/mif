//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_DB_ISTATEMENT_H__
#define __MIF_DB_ISTATEMENT_H__

// STD
#include <list>

// MIF
#include "mif/db/irecordset.h"
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Db
    {

        struct IStatement
            : public Service::Inherit<Service::IService>
        {
            using Parameters = std::list<std::string/*value*/>;

            virtual IRecordsetPtr Execute(Parameters const &parameters = {}) = 0;
        };

        using IStatementPtr = Service::TServicePtr<IStatement>;

    }   // namespace Db
}   // namespace Mif

#endif  // !__MIF_DB_ISTATEMENT_H__
