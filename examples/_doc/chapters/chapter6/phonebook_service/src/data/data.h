#ifndef __PHONEBOOK_DATA_DATA_H__
#define __PHONEBOOK_DATA_DATA_H__

// STD
#include <cstdint>
#include <string>

namespace Phonebook
{
    namespace Data
    {

        struct Status
        {
            std::int32_t code = 0;
            std::string message;
        };

        struct Info
        {
            std::string service;
            std::string node;
            std::string timestamp;
            Status status;
        };

        struct Header
        {
            Info meta;
        };

    }   // namespace Data
}   // namespace Phonebook

#endif  // !__PHONEBOOK_DATA_DATA_H__
