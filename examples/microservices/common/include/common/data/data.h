//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MICROSERVICES_COMMON_DATA_DATA_H__
#define __MICROSERVICES_COMMON_DATA_DATA_H__

// STD
#include <cstdint>
#include <map>
#include <string>

namespace Common
{
    namespace Data
    {

        using ID = std::string;

        enum class Position
        {
            Unknown,
            Developer,
            Manager
        };

        struct Employee
        {
            std::string name;
            std::string lastName;
            std::uint32_t age = 0;
            std::string email;
            Position position = Position::Unknown;
            double rate = 0.0;
        };

        using Employees = std::map<ID, Employee>;

    }   // namespace Data
}   // namespace Common

#endif  // !__MICROSERVICES_COMMON_DATA_DATA_H__
