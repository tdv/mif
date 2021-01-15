//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __COMPLEX_TYPE_COMMON_DATA_DATA_H__
#define __COMPLEX_TYPE_COMMON_DATA_DATA_H__

// STD
#include <cstdint>
#include <map>
#include <string>

namespace Service
{
    namespace Data
    {

        using ID = std::string;

        struct Human
        {
            std::string name;
            std::string lastName;
            std::uint32_t age = 0;
        };

        enum class Position
        {
            Unknown,
            Developer,
            Manager
        };

        struct Employee
            : public Human
        {
            Position position = Position::Unknown;
        };

        using Employees = std::map<ID, Employee>;

    }   // namespace Data
}   // namespace Service

#endif  // !__COMPLEX_TYPE_COMMON_DATA_DATA_H__
