//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MICROSERVICES_COMMON_INTERFACE_IEMPPLOYEE_STORAGE_H__
#define __MICROSERVICES_COMMON_INTERFACE_IEMPPLOYEE_STORAGE_H__

// STD
#include <cstdint>

// MIF
#include <mif/service/iservice.h>

// THIS
#include "common/data/data.h"

namespace Common
{

    struct IEmployeeStorage
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
        virtual Data::ID Create(Data::Employee const &employee) = 0;
        virtual Data::Employee Read(Data::ID const &id) const = 0;
        virtual void Update(Data::ID const &id, Data::Employee const &employee) = 0;
        virtual void Delete(Data::ID const &id) = 0;

        // CRUD extension method
        virtual Data::Employees List(std::size_t const *offset, std::size_t const *limit) const = 0;
    };

}   // namespace Common

#endif  // !__MICROSERVICES_COMMON_INTERFACE_IEMPPLOYEE_STORAGE_H__
