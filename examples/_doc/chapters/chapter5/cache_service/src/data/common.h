#ifndef __CACHESERVICE_DATA_COMMON_H__
#define __CACHESERVICE_DATA_COMMON_H__

// STD
#include <set>
#include <string>

namespace CacheService
{
    namespace Data
    {

        using ID = std::string;
        using IDs = std::set<ID>;

        enum class Role
        {
            Admin,
            User
        };

        using Roles = std::set<Role>;

        struct Profile
        {
            ID id;
            std::string login;
            std::string password;
            std::string name;
            Roles roles;
            IDs buckets;
        };

    }   // namespace Data
}   // namespace CacheService

#endif  // !__CACHESERVICE_DATA_COMMON_H__
