#ifndef __CACHESERVICE_DATA_COMMON_H__
#define __CACHESERVICE_DATA_COMMON_H__

// STD
#include <map>
#include <set>
#include <string>

// BOOST
#include <boost/date_time/posix_time/posix_time.hpp>

namespace CacheService
{
    namespace Data
    {

        using ID = std::string;
        using IDs = std::set<ID>;
        using StringMap = std::map<std::string, std::string>;
        using Timestamp = boost::posix_time::ptime;

        enum class Role
        {
            Admin,
            User
        };

        using Roles = std::set<Role>;

        struct Profile
        {
            std::string login;
            std::string password;
            std::string name;
            Roles roles;
            IDs buckets;
        };

        struct Session
        {
            ID id;
            Timestamp expires;
        };

    }   // namespace Data
}   // namespace CacheService

#endif  // !__CACHESERVICE_DATA_COMMON_H__
