//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>
#include <thread>
#include <utility>

// BOOST
#include <boost/asio.hpp>

// MIF
#include "mif/net/tcp_clients.h"

// THIS
#include "mif/common/log.h"
#include "detail/tcp_session.h"

namespace Mif
{
    namespace Net
    {

        class TCPClients::Impl final
        {
        public:
            Impl(IClientFactoryPtr factory)
            try
                : m_factory(factory)
                , m_work(m_ioService)
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
            }
            catch (std::exception const &e)
            {
                throw std::runtime_error{"[Mif::Net::TCPClients::Impl] Failed tp start clients. "
                    "Error: " + std::string{e.what()}};
            }
            catch (...)
            {
                throw std::runtime_error{"[Mif::Net::TCPClients::Impl] Failed tp start clients. Error: unknown."};
            }

            ~Impl()
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
                                MIF_LOG(Error) << "[Mif::Net::TCPClients::Impl] Failed tp post 'stop' to clients. "
                                    << "Error: " << e.what();
                            }
                        }
                    );

                m_thread->join();
            }
            catch (std::exception const &e)
            {
                MIF_LOG(Error) << "[Mif::Net::TCPClients::Impl] Failed tp stop clients. "
                    << "Error: " << e.what();
            }
            catch (...)
            {
                MIF_LOG(Error) << "[Mif::Net::TCPClients::Impl] Failed tp stop clients. Error: unknown.";
            }

            IClientFactory::ClientPtr RunClient(std::string const &host, std::string const &port)
            {
                try
                {
                    boost::asio::ip::tcp::socket socket{m_ioService};
                    boost::asio::ip::tcp::resolver resolver{m_ioService};
                    boost::asio::connect(socket, resolver.resolve({host, port}));
                    return std::make_shared<Detail::TCPSession>(std::move(socket), *m_factory)->Start();
                }
                catch (std::exception const &e)
                {
                    throw std::runtime_error{"[Mif::Net::TCPClients::Impl::RunClient] Failed tp run client. "
                        "Error: " + std::string{e.what()}};
                }
            }

        private:
            std::shared_ptr<IClientFactory> m_factory;
            boost::asio::io_service m_ioService;
            std::unique_ptr<std::thread> m_thread;
            boost::asio::io_service::work m_work;
        };


        TCPClients::TCPClients(IClientFactoryPtr factory)
            : m_impl{new TCPClients::Impl{factory}}
        {
        }

        TCPClients::~TCPClients()
        {
        }

        IClientFactory::ClientPtr TCPClients::RunClient(std::string const &host, std::string const &port)
        {
            return m_impl->RunClient(host, port);
        }


    }   // namespace Net
}   // namespace Mif
