//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MICROSERVICES_COMMON_INTERFACE_IEMPPLOYEE_STORAGE_H__
#define __MICROSERVICES_COMMON_INTERFACE_IEMPPLOYEE_STORAGE_H__

// STD
#include <cstdint>
#include <limits>

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
        virtual Data::Employees List(std::size_t offset = std::numeric_limits<std::size_t>::max(),
                std::size_t limit = std::numeric_limits<std::size_t>::max()) const = 0;
    };

}   // namespace Common

#endif  // !__MICROSERVICES_COMMON_INTERFACE_IEMPPLOYEE_STORAGE_H__
