//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MICROSERVICES_SERVICE_DATA_RESPONSES_H__
#define __MICROSERVICES_SERVICE_DATA_RESPONSES_H__

// STD
#include <cstdint>
#include <map>
#include <string>

// COMMON
#include "common/data/data.h"

namespace Service
{
    namespace Data
    {
        namespace Response
        {

            struct Status
            {
                std::uint32_t code = 0;
                std::string message;
            };

            struct Info
            {
                std::string timestamp;
                std::string node;
                Status status;
            };

            struct Void
            {
                Info meta;
            };

            struct Id
            {
                Info meta;
                Common::Data::ID data;
            };

            struct Employee
            {
                Info meta;
                Common::Data::Employee data;
            };

            struct Employees
            {
                Info meta;
                Common::Data::Employees data;
            };

        }   // namespace Response
    }   // namespace Data
}   // namespace Service

#endif  // !__MICROSERVICES_SERVICE_DATA_RESPONSES_H__
