// STD
#include <iostream>
#include <stdexcept>
#include <string>

// BOOST
#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>

// MIF
#include "mif/common/thread_pool.h"

namespace Mif
{
    namespace Common
    {

        class ThreadPool::Impl final
        {
        public:
            Impl(std::uint16_t count)
                : m_work{new boost::asio::io_service::work{m_ioService}}
            {
                if (!count)
                    throw std::invalid_argument{"[Mif::Common::ThreadPool::Impl] Thread count must be more than 0."};

                std::exception_ptr exception{};
                for ( ; count ; --count)
                {
                    m_threads.create_thread([this, &exception] ()
                            {
                                try
                                {
                                    m_ioService.run();
                                }
                                catch (std::exception const &)
                                {
                                    exception = std::current_exception();
                                }
                            }
                        );

                    if (exception)
                        break;
                }

                if (exception)
                {
                    if (count)
                        Stop();

                    try
                    {
                        std::rethrow_exception(exception);
                    }
                    catch (std::exception const &e)
                    {
                        throw std::runtime_error{"[Mif::Common::ThreadPool::Impl] Failed to run thread pool. "
                            "Error: " + std::string{e.what()}};
                    }
                }
            }

            ~Impl()
            {
                Stop();
            }

            void Post(Task task)
            {
                m_ioService.post(task);
            }

        private:
            boost::asio::io_service m_ioService;
            std::unique_ptr<boost::asio::io_service::work> m_work;
            boost::thread_group m_threads;

            void Stop()
            {
                try
                {
                    m_work.reset();
                    m_ioService.post([this] () { m_ioService.stop(); });
                    m_threads.join_all();
                }
                catch (std::exception const &e)
                {
                    std::cerr << "[Mif::Common::ThreadPool::Impl::Stop] Failed to stop thread pool. Error: " << e.what() << std::endl;
                }
            }
        };


        ThreadPool::ThreadPool(std::uint16_t count)
            : m_impl{new Impl{count}}
        {
        }

        ThreadPool::~ThreadPool()
        {
        }

        void ThreadPool::Post(Task task)
        {
            m_impl->Post(std::move(task));
        }

    }   // namespace Common
}   // namespace Mif
