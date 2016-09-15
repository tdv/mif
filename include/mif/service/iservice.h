#ifndef __MIF_SERVICE_ISERVICE_H__
#define __MIF_SERVICE_ISERVICE_H__

// STD
#include <memory>

namespace Mif
{
    namespace Service
    {

        struct IService
            : public std::enable_shared_from_this<IService>
        {
            virtual ~IService() = default;
        };

        using IServicePtr = std::shared_ptr<IService>;

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_ISERVICE_H__
