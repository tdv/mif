// STD
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <utility>

// BOOST
#include <boost/asio.hpp>

// THIS
#include "tcp_session.h"

namespace Mif
{
    namespace Net
    {
        namespace Detail
        {

            TCPSession::TCPSession(boost::asio::ip::tcp::socket socket,
                Common::ThreadPool &workers, ISubscriberFactory &factory)
                : m_socket(std::move(socket))
                , m_workers(workers)
                , m_factory(factory)
            {
            }

            void TCPSession::Start()
            {
                m_subscriber = m_factory.Create(std::weak_ptr<IControl>(shared_from_this()),
                    std::weak_ptr<IPublisher>(shared_from_this()));

                DoRead();
            }


            void TCPSession::Publish(Common::Buffer buffer)
            {
                try
                {
                    auto self(shared_from_this());

                    m_socket.get_io_service().post([self, buffer] ()
                            {
                                boost::asio::async_write(self->m_socket, boost::asio::buffer(buffer.second.get(), buffer.first),
                                        [self, buffer] (boost::system::error_code error, std::size_t /*length*/)
                                        {
                                            if (error)
                                            {
                                                // TODO: error to log
                                                self->CloseMe();
                                                std::cerr << "OnWrite error. Error: " << error.message() << std::endl;
                                            }
                                        }
                                    );
                            }
                        );
                }
                catch (std::exception const &)
                {
                    // TODO: to log
                    CloseMe();
                }
            }

            void TCPSession::CloseMe()
            {
                auto self = shared_from_this();
                m_socket.get_io_service().post([self] ()
                        {
                            try
                            {
                                self->m_socket.close();
                            }
                            catch (std::exception const &e)
                            {
                                std::cerr << "CloseMe error: " << e.what() << std::endl;
                                // TODO: to log
                            }
                        }
                    );
            }

            void TCPSession::DoRead()
            {
                std::size_t const bytes = 4096; // TODO: from params
                auto buffer = std::make_pair(bytes, boost::shared_array<char>(new char [bytes]));
                auto self(shared_from_this());
                m_socket.async_read_some(boost::asio::buffer(buffer.second.get(), buffer.first),
                    [self, buffer] (boost::system::error_code error, std::size_t length)
                    {
                        try
                        {
                            if (!error)
                            {
                                self->m_workers.Post([self, length, buffer] ()
                                        {
                                            try
                                            {
                                                self->m_subscriber->OnData(std::make_pair(length, std::move(buffer.second)));
                                            }
                                            catch (std::exception const &e)
                                            {
                                                // TODO: to log
                                                std::cerr << "Failed to process data. Error: " << e.what() << std::endl;
                                            }
                                        }
                                    );
                                self->DoRead();
                            }
                            else
                            {
                                // TODO: to log
                                self->CloseMe();
                                std::cerr << "Failed to process data. Error: " << error.message() << std::endl;
                            }
                        }
                        catch (std::exception const &e)
                        {
                            // TODO: to lgg
                            self->CloseMe();
                            std::cerr << "Failed to process data. Error: " << e.what() << std::endl;
                        }
                    }
                );
            }

        }   // namespace Detail
    }   // namespace Net
}   // namespace Mif
