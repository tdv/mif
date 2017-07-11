//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __COMPLEX_TYPE_COMMON_INTERFACE_IMYCOMPANY_H__
#define __COMPLEX_TYPE_COMMON_INTERFACE_IMYCOMPANY_H__

// MIF
#include <mif/service/iservice.h>

// THIS
#include "common/data/data.h"

namespace Service
{

    struct IMyCompany
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
        virtual Data::ID AddEmployee(Data::Employee const &employee) = 0;
        virtual void RemoveAccount(Data::ID const &id) = 0;
        virtual Data::Employees GetEmployees() const = 0;
    };

}   // namespace Service

#endif  // !__COMPLEX_TYPE_COMMON_INTERFACE_IMYCOMPANY_H__
