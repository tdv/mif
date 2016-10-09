//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>
#include <stdexcept>
#include <string>

// BOOST
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/thread.hpp>

// MIF
#include "mif/common/thread_pool.h"

namespace Mif
{
    namespace Common
    {
        namespace Detail
        {
            namespace
            {

                class OrderedPoster final
                    : public IThreadPool
                {
                public:
                    OrderedPoster(std::shared_ptr<IThreadPool> threadPool, boost::asio::io_service &ioService)
                        : m_threadPool{threadPool}
                        , m_strand{ioService}
                    {
                    }

                private:
                    std::shared_ptr<IThreadPool> m_threadPool;
                    boost::asio::io_service::strand m_strand;

                    // IThreadPool
                    virtual void Post(Task task) override final
                    {
                        m_strand.post(std::move(task));
                    }

                    virtual std::shared_ptr<IThreadPool> CreateOrderedPoster() override final
                    {
                        return m_threadPool->CreateOrderedPoster();
                    }
                };

                class ThreadPool final
                    : public IThreadPool
                {
                public:
                    ThreadPool(std::uint16_t count)
                        : m_work{new boost::asio::io_service::work{m_ioService}}
                    {
                        if (!count)
                            throw std::invalid_argument{"[Mif::Common::ThreadPool] Thread count must be more than 0."};

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
                                throw std::runtime_error{"[Mif::Common::ThreadPool] Failed to run thread pool. "
                                    "Error: " + std::string{e.what()}};
                            }
                        }
                    }

                    ~ThreadPool()
                    {
                        Stop();
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
                            std::cerr << "[Mif::Common::ThreadPool::Stop] Failed to stop thread pool. Error: " << e.what() << std::endl;
                        }
                    }

                    // IThreadPool
                    virtual void Post(Task task) override final
                    {
                        m_ioService.post(std::move(task));
                    }

                    virtual std::shared_ptr<IThreadPool> CreateOrderedPoster() override final
                    {
                        return std::make_shared<OrderedPoster>(std::static_pointer_cast<ThreadPool>(shared_from_this()), m_ioService);
                    }
                };

            }   // namespace
        }   // namespace Detail

        std::shared_ptr<IThreadPool> CreateThreadPool(std::uint16_t threadCount)
        {
            return std::make_shared<Detail::ThreadPool>(threadCount);
        }

    }   // namespace Common
}   // namespace Mif
