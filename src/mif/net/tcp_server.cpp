// STD
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <thread>

// BOOST
#include <boost/asio.hpp>

// MIF
#include "mif/net/tcp_server.h"

// THIS
#include "detail/tcp_session.h"

namespace Mif
{
    namespace Net
    {

        class TCPServer::Impl final
        {
        public:
            Impl(std::string const &host, std::string const &port,
                std::uint16_t workers, std::shared_ptr<IClientFactory> factory)
            try
                : m_factory{factory}
                , m_workers{Common::CreateThreadPool(workers)}
                , m_acceptor{m_ioService,
                        [this, &host, &port] () -> boost::asio::ip::tcp::endpoint
                        {
                            boost::asio::ip::tcp::resolver resolver{m_ioService};
                            return *resolver.resolve({host, port});
                        } ()
                    }
                , m_socket{m_ioService}
                , m_work{m_ioService}
            {
                std::exception_ptr exception{};
                m_thread.reset(new std::thread([this, &exception] ()
                        {
                            try
                            {
                                m_ioService.run();
                            }
                            catch (std::exception const &e)
                            {
                                exception = std::current_exception();
                            }
                        }
                    )
                );

                if (exception)
                    std::rethrow_exception(exception);

                DoAccept();
            }
            catch (std::exception const &e)
            {
                throw std::runtime_error{"[Mif::Net::TCPServer::Impl] Failed to start server. "
                    "Error: " + std::string{e.what()}};
            }
            catch (...)
            {
                throw std::runtime_error{"[Mif::Net::TCPServer::Impl] Failed to start server. Error: unknown."};
            }

            virtual ~Impl()
            {
                try
                {
                    m_ioService.post([this] ()
                            {
                                try
                                {
                                    m_ioService.stop();
                                }
                                catch (std::exception const &e)
                                {
                                    std::cerr << "[Mif::Net::~TCPServer::Impl] Failed to post 'stop' to server object. Error: " << e.what() << std::endl;
                                }
                            }
                        );

                    m_thread->join();
                }
                catch (std::exception const &e)
                {
                    std::cerr << "[Mif::Net::~TCPServer::Impl] Failed to stop server. Error: " << e.what() << std::endl;
                }
            }

        private:
            std::shared_ptr<IClientFactory> m_factory;
            std::shared_ptr<Common::IThreadPool> m_workers;
            std::unique_ptr<std::thread> m_thread;
            boost::asio::io_service m_ioService;
            boost::asio::ip::tcp::acceptor m_acceptor;
            boost::asio::ip::tcp::socket m_socket;
            boost::asio::io_service::work m_work;

            void DoAccept()
            {
                m_acceptor.async_accept(m_socket,
                        [this] (boost::system::error_code error)
                        {
                            try
                            {
                                if (!error)
                                    std::make_shared<Detail::TCPSession>(std::move(m_socket), m_workers, *m_factory)->Start();
                                else
                                {
                                    std::cerr << "[Mif::Net::TCPServer::Impl::DoAccept] Failed tp accept connection. "
                                              << "Error: " << error.message() << std::endl;
                                }
                                DoAccept();
                            }
                            catch (std::exception const &e)
                            {
                                std::cerr << "[Mif::Net::TCPServer::Impl::DoAccept] Failed to accept connection. Error: " << e.what() << std::endl;
                            }
                        }
                    );
            }
        };

        TCPServer::TCPServer(std::string const &host, std::string const &port,
            std::uint16_t workers, std::shared_ptr<IClientFactory> factory)
            : m_impl{new Impl{host, port, workers, factory}}
        {
        }

        TCPServer::~TCPServer()
        {
        }

    }   // namespace Net
}   // namespace Mif
