#ifndef __PHONEBOOK_EXCEPTION_H__
#define __PHONEBOOK_EXCEPTION_H__

// STD
#include <stdexcept>

namespace Phonebook
{
    namespace Exception
    {

        class Unauthorized
            : public std::runtime_error
        {
        public:
            using runtime_error::runtime_error;
        };


        class NotFound
            : public std::runtime_error
        {
        public:
            using runtime_error::runtime_error;
        };

    }   // namespace Exception
}   // namespace Phonebook

#endif  // !__PHONEBOOK_EXCEPTION_H__
