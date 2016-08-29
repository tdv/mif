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

        class ThreadPool final
        {
        public:
            using Task = std::function<void ()>;

            ThreadPool(std::uint16_t count);
            ~ThreadPool();

            void Post(Task task);

        private:
            class Impl;
            std::unique_ptr<Impl> m_impl;
        };

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_THREAD_POOL_H__
