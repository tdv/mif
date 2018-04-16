#ifndef __CACHESERVICE_DATA_API_H__
#define __CACHESERVICE_DATA_API_H__

// STD
#include <cstdint>
#include <map>
#include <string>

// THIS
#include "data/common.h"

namespace CacheService
{
    namespace Data
    {
        namespace Api
        {
            namespace Response
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

                struct Data
                {
                    Info meta;
                    std::string data;
                };

                struct BucketKeys
                {
                    Info meta;
                    std::map<ID, IDs> data;
                };

            }   // namespace Response
        }   // namespace Api
    }   // namespace Data
}   // namespace CacheService

#endif  // !__CACHESERVICE_DATA_API_H__
