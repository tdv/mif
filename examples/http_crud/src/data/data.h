//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __HTTP_CRUD_DATA_DATA_H__
#define __HTTP_CRUD_DATA_DATA_H__

// STD
#include <cstdint>
#include <map>
#include <string>

namespace Service
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
                ID data;
            };

            struct Employee
            {
                Info meta;
                Data::Employee data;
            };

            struct Employees
            {
                Info meta;
                Data::Employees data;
            };

        }   // namespace Response
    }   // namespace Data
}   // namespace Service

#endif  // !__HTTP_CRUD_DATA_DATA_H__
