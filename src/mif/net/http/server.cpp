//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

// STD
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>
#include <utility>

// BOOST
#include <boost/beast/core.hpp>
#include <boost/make_unique.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>

// MIF
#include "mif/common/log.h"
#include "mif/net/http/server.h"

// THIS
#include "detail/session.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            class Listener final
                : public std::enable_shared_from_this<Listener>
            {
            public:
                Listener(boost::asio::io_context &ioc,
                        boost::asio::ip::tcp::endpoint endpoint,
                        Detail::Session::Params const &sessionParams)
                    : m_ioc{ioc}
                    , m_acceptor{boost::asio::make_strand(m_ioc)}
                    , m_sessionParams{sessionParams}
                {
                    boost::beast::error_code ec;

                    m_acceptor.open(endpoint.protocol(), ec);

                    if (ec)
                    {
                        throw std::runtime_error{"[Mif::Net::Http::Listener] "
                                "Failed to open acceptor. Error: " + ec.message()};
                    }

                    m_acceptor.set_option(boost::asio::socket_base::reuse_address{true}, ec);

                    if (ec)
                    {
                        throw std::runtime_error{"[Mif::Net::Http::Listener] "
                                "Failed to reuse address. Error: " + ec.message()};
                    }

                    m_acceptor.bind(endpoint, ec);

                    if (ec)
                    {
                        throw std::runtime_error{"[Mif::Net::Http::Listener] "
                                "Failed to bind socket. Error: " + ec.message()};
                    }

                    m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);

                    if (ec)
                    {
                        throw std::runtime_error{"[Mif::Net::Http::Listener] "
                                "Failed to start listening to socket. Error: " + ec.message()};
                    }
                }

                void Run()
                {
                    boost::asio::dispatch(m_acceptor.get_executor(),
                            boost::beast::bind_front_handler(&Listener::Accept,
                            shared_from_this())
                        );
                }

            private:
                boost::asio::io_context &m_ioc;
                boost::asio::ip::tcp::acceptor m_acceptor;

                Detail::Session::Params m_sessionParams;

                void Accept()
                {
                    m_acceptor.async_accept(boost::asio::make_strand(m_ioc),
                            boost::beast::bind_front_handler(&Listener::OnAccept,
                            shared_from_this())
                        );
                }

                void OnAccept(boost::beast::error_code ec,
                        boost::asio::ip::tcp::socket socket)
                {
                    if (ec)
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Listener::OnAccept] "
                                << "Failed to accept connection. Error: " << ec.message();
                    }
                    else
                    {
                        auto session = std::make_shared<Detail::Session>(std::move(socket),
                                m_sessionParams);
                        session->Run();
                    }

                    Accept();
                }
            };

            class Server::Impl final
            {
            public:
                Impl(std::string const &host, std::string const &port,
                        std::uint16_t workers, ServerHandlers const &handlers, Methods const &allowedMethods,
                        std::size_t headersSize, std::size_t bodySize, std::size_t chunkSize,
                        std::size_t requestTimeout, std::size_t pipelineLimit)
                    : m_ioc{std::max<int>(1, workers) + 1}
                {
                    Detail::Session::Params sessionParams;

                    auto const defaultValue = static_cast<std::size_t>(-1);
                    if (headersSize != defaultValue)
                        sessionParams.headersSize = headersSize;
                    if (bodySize != defaultValue)
                        sessionParams.bodySize = bodySize;
                    if (chunkSize != defaultValue)
                        sessionParams.chunkSize = chunkSize;
                    if (requestTimeout != defaultValue)
                        sessionParams.requestTimeout = std::chrono::microseconds{requestTimeout};
                    if (pipelineLimit != defaultValue)
                        sessionParams.pipelineLimit = pipelineLimit;

                    sessionParams.allowedMethods = allowedMethods;
                    sessionParams.handlers = handlers;

                    auto address = boost::asio::ip::make_address(host);
                    auto endpoint = boost::asio::ip::tcp::endpoint{std::move(address),
                            static_cast<std::uint16_t>(std::stoi(port))};

                    auto listener = std::make_shared<Listener>(m_ioc, std::move(endpoint),
                            std::move(sessionParams));

                    listener->Run();

                    for (auto i = std::max<int>(1, workers) + 1 ; i ; --i)
                    {
                        m_threads.create_thread([this]
                                {
                                    while (m_isRun)
                                    {
                                        try
                                        {
                                            m_ioc.run();
                                        }
                                        catch (std::exception const &e)
                                        {
                                            MIF_LOG(Fatal) << "[Mif::Net::Http::Server::Impl] "
                                                    << "Failed to run ioc. Error: " << e.what();

                                            std::abort();
                                        }

                                        if (m_isRun)
                                        {
                                            MIF_LOG(Warning) << "[Mif::Net::Http::Server::Impl] "
                                                    << "Ioc has been interrupted and run again.";
                                            std::this_thread::sleep_for(std::chrono::milliseconds{20});;
                                        }
                                    }
                                }
                            );
                    }
                }

                ~Impl()
                {
                    m_isRun = false;

                    try
                    {
                        m_ioc.stop();
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Fatal) << "[Mif::Net::Http::Server::~Impl] "
                            << "Failed to stop ioc. Error: " << e.what();

                        std::abort();
                    }

                    try
                    {
                        m_threads.join_all();
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Fatal) << "[Mif::Net::Http::Server::~Impl] "
                            << "Failed to join all threads. Error: " << e.what();

                        std::abort();
                    }
                }

            private:
                std::atomic<bool> m_isRun{true};
                boost::thread_group m_threads;
                boost::asio::io_context m_ioc;
            };


            Server::Server(std::string const &host, std::string const &port,
                std::uint16_t workers, Methods const &allowedMethods, ServerHandlers const &handlers,
                std::size_t headersSize, std::size_t bodySize, std::size_t chunkSize, std::size_t requestTimeout,
                std::size_t pipelineLimit)
                : m_impl{boost::make_unique<Impl>(host, port, workers, handlers, allowedMethods, headersSize, bodySize,
                        chunkSize, requestTimeout, pipelineLimit)}
            {
            }

            Server::~Server()
            {
            }

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
