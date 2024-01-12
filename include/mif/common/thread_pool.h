//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_THREAD_POOL_H__
#define __MIF_COMMON_THREAD_POOL_H__

// STD
#include <cstdint>
#include <memory>
#include <functional>

namespace Mif
{
    namespace Common
    {

        struct IThreadPool
            : public std::enable_shared_from_this<IThreadPool>
        {
            using Task = std::function<void ()>;

            virtual ~IThreadPool() = default;

            virtual void Post(Task task) = 0;
            virtual std::shared_ptr<IThreadPool> CreateOrderedPoster() = 0;
        };

        using IThreadPoolPtr = std::shared_ptr<IThreadPool>;

        std::shared_ptr<IThreadPool> CreateThreadPool(std::uint16_t threadCount);

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_THREAD_POOL_H__
