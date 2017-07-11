//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <cstdint>
#include <stdexcept>
#include <utility>

// BOOST
#include <boost/asio.hpp>

// MIF
#include "mif/common/log.h"

// THIS
#include "tcp_session.h"

namespace Mif
{
    namespace Net
    {
        namespace Detail
        {

            TCPSession::TCPSession(boost::asio::ip::tcp::socket socket, IClientFactory &factory)
                : m_socket{std::move(socket)}
                , m_factory{factory}
            {
            }

            IClientFactory::ClientPtr TCPSession::Start()
            {
                m_client = m_factory.Create(std::weak_ptr<IControl>(shared_from_this()),
                    std::weak_ptr<IPublisher>(shared_from_this()));

                DoRead();

                return m_client;
            }


            void TCPSession::Publish(Common::Buffer buffer)
            {
                try
                {
                    auto self = shared_from_this();

                    auto data = std::make_shared<Common::Buffer>(std::move(buffer));

                    m_socket.get_io_service().post([self, data] ()
                            {
                                boost::asio::async_write(self->m_socket, boost::asio::buffer(*data),
                                        [self, data] (boost::system::error_code error, std::size_t /*length*/)
                                        {
                                            if (error)
                                            {
                                                MIF_LOG(Warning) << "[Mif::Net::Detail::TCPSession::Publisher]. "
                                                    << "Failed to write data. Error: " << error.message();
                                                self->CloseMe();
                                            }
                                        }
                                    );
                            }
                        );
                }
                catch (std::exception const &e)
                {
                    CloseMe();
                    MIF_LOG(Warning) << "[Mif::Net::Detail::TCPSession::Publisher]. "
                        << "Failed to publish data. Error: " << e.what();
                }
            }

            void TCPSession::CloseMe()
            {
                auto self = shared_from_this();
                m_socket.get_io_service().post([self] ()
                        {
                            try
                            {
                                self->m_client->OnClose();
                            }
                            catch (std::exception const &e)
                            {
                                MIF_LOG(Warning) << "[Mif::Net::Detail::TCPSession::CloseMe]. "
                                    << "Failed to call OnClose for client. Error: " << e.what();
                            }

                            try
                            {
                                self->m_socket.close();
                            }
                            catch (std::exception const &e)
                            {
                                MIF_LOG(Warning) << "[Mif::Net::Detail::TCPSession::CloseMe]. "
                                    << "Failed to close client socket. Error: " << e.what();
                            }
                        }
                    );
            }

            void TCPSession::DoRead()
            {
                auto buffer = std::make_shared<Common::Buffer>(8192);   // TODO: parametrize it
                auto self = shared_from_this();
                m_socket.async_read_some(boost::asio::buffer(*buffer),
                    [self, buffer] (boost::system::error_code error, std::size_t length)
                    {
                        try
                        {
                            if (!error)
                            {
                                self->m_socket.get_io_service().post([self, length, buffer] ()
                                        {
                                            try
                                            {
                                                auto begin = std::begin(*buffer);
                                                auto end = begin;
                                                std::advance(end, length);
                                                self->m_client->OnData({begin, end});
                                            }
                                            catch (std::exception const &e)
                                            {
                                                MIF_LOG(Warning) << "[Mif::Net::Detail::TCPSession::DoRead]. "
                                                    << "Failed to process data. Error: " << e.what();
                                            }
                                        }
                                    );
                                self->DoRead();
                            }
                            else
                            {
                                self->CloseMe();
                                if (error.value() != boost::asio::error::eof)
                                {
                                    MIF_LOG(Warning) << "[Mif::Net::Detail::TCPSession::DoRead]. "
                                        << "Failed to read data. Error: " << error.message();
                                }
                            }
                        }
                        catch (std::exception const &e)
                        {
                            self->CloseMe();
                            MIF_LOG(Warning) << "[Mif::Net::Detail::TCPSession::DoRead]. "
                                << "Failed to post task on data processing. Error: " << e.what();
                        }
                    }
                );
            }

        }   // namespace Detail
    }   // namespace Net
}   // namespace Mif
