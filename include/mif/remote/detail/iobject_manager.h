#ifndef __MIF_REMOTE_DETAIL_IOBJECT_MANAGER_H__
#define __MIF_REMOTE_DETAIL_IOBJECT_MANAGER_H__

// STD
#include <string>

namespace Mif
{
    namespace Remote
    {
        namespace Detail
        {

            struct IObjectManager
            {
                virtual ~IObjectManager() = default;
                virtual std::string CreateObject(std::string const &classId) = 0;
                virtual void DestroyObject(std::string const &instanceId) = 0;
            };

        }   // namespace Detail
    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_DETAIL_IOBJECT_MANAGER_H__
