#ifndef __CACHESERVICE_EXCEPTION_H__
#define __CACHESERVICE_EXCEPTION_H__

// STD
#include <stdexcept>

namespace CacheService
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
}   // namespace CacheService

#endif  // !__CACHESERVICE_EXCEPTION_H__
