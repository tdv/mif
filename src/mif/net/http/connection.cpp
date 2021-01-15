//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

// STD
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

// BOOST
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/make_unique.hpp>

// MIF
#include "mif/common/log.h"
#include "mif/common/unused.h"
#include "mif/net/http/connection.h"

// THIS
#include "detail/input_pack.h"
#include "detail/output_pack.h"
#include "detail/utility.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace
            {

                class Session final
                    : public std::enable_shared_from_this<Session>
                {
                public:
                    using RequestBodyType = boost::beast::http::buffer_body;
                    using Request = boost::beast::http::request<RequestBodyType>;

                    Session(boost::asio::io_context &ioc, Connection::Params const &params,
                            ClientHandler &handler, Connection::OnCloseHandler &onClose)
                        : m_ioc{ioc}
                        , m_stream{boost::asio::make_strand(m_ioc)}
                        , m_params{params}
                        , m_handler{handler}
                        , m_onClose{onClose}
                    {
                    }

                    ~Session()
                    {
                    }

                    void Run(boost::asio::ip::tcp::resolver::results_type const &destinations,
                            Connection::Params const &params, Connection::IOutputPackPtr pack)
                    {
                        if (params.connectionTimeout)
                            m_stream.expires_after(*params.connectionTimeout);

                        m_stream.async_connect(destinations, boost::beast::bind_front_handler(
                                &Session::OnConnect, shared_from_this(), std::move(pack)
                            ));
                    }

                    void Post(Connection::IOutputPackPtr pack)
                    {
                        if (m_isClosed)
                        {
                            throw std::logic_error("[Mif::Net::Http::Session::Post] "
                                    "The connection has not been opened or has already closed.");
                        }

                        auto &request = dynamic_cast<Detail::OutputPack<Request> &>(*pack).GetData();
                        request.keep_alive(false);

                        if (m_params.requestTimeout)
                            m_stream.expires_after(*m_params.requestTimeout);

                        boost::beast::http::async_write(m_stream, request,
                                boost::beast::bind_front_handler(&Session::OnWrite,
                                shared_from_this(), pack, std::string{request.target()}));
                    }

                    bool IsClosed() const
                    {
                        return m_isClosed;
                    }

                private:
                    std::atomic<bool> m_isClosed{false};

                    boost::asio::io_context &m_ioc;
                    boost::beast::tcp_stream m_stream;
                    Connection::Params const &m_params;
                    ClientHandler &m_handler;
                    Connection::OnCloseHandler &m_onClose;

                    using BufferType = boost::beast::flat_buffer;
                    using BufferPtr = std::shared_ptr<BufferType>;

                    using ResponseBodyType = boost::beast::http::vector_body<char>;
                    using ResponseType = boost::beast::http::response<ResponseBodyType>;
                    using ResponsePtr = std::shared_ptr<ResponseType>;

                    void OnConnect(Connection::IOutputPackPtr pack, boost::beast::error_code const &ec,
                            boost::asio::ip::tcp::resolver::results_type::endpoint_type const &endpoint)
                    {
                        Mif::Common::Unused(endpoint);

                        if (ec)
                        {
                            m_isClosed = true;

                            MIF_LOG(Error) << "[Mif::Net::Http::Session::OnConnect] "
                                    << "Failed to connect to endpoint. Error: " << ec.message();

                            return;
                        }

                        m_isClosed = false;

                        Post(std::move(pack));
                    }

                    void OnWrite(Connection::IOutputPackPtr pack, std::string const target,
                            boost::beast::error_code const &ec, std::size_t bytes)
                    {
                        Mif::Common::Unused(pack, bytes);

                        if (ec)
                        {
                            MIF_LOG(Error) << "[Mif::Net::Http::Session::OnWrite] "
                                    << "Failed to send request. Error: " << ec.message();

                            Close();
                            return;;
                        }

                        auto buffer = std::make_shared<BufferType>();
                        auto response = std::make_shared<ResponseType>();

                        boost::beast::http::async_read(m_stream, *buffer, *response,
                                boost::beast::bind_front_handler(&Session::OnRead,
                                shared_from_this(), buffer, response, target
                            ));
                    }

                    void OnRead(BufferPtr buffer, ResponsePtr response,
                            std::string const target,
                            boost::beast::error_code const &ec, std::size_t bytes)
                    {
                        Mif::Common::Unused(buffer, bytes);

                        if (ec)
                        {
                            MIF_LOG(Error) << "[Mif::Net::Http::Session::OnRead] "
                                    << "Failed to receive response. Error: " << ec.message();

                            Close();
                            return;
                        }

                        OnResponse(*response, target);

                        //if (!response->keep_alive())
                            Close();
                    }

                    void Close()
                    {
                        boost::beast::error_code ec;

                        m_stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

                        if (ec)
                        {
                            MIF_LOG(Error) << "[Mif::Net::Http::Session::OnClose] "
                                    << "Failed to close connection. Error: " << ec.message();
                        }

                        OnClose();
                    }

                    void OnClose() noexcept
                    {
                        try
                        {
                            m_isClosed = true;
                            m_onClose();
                        }
                        catch (std::exception const &e)
                        {
                            MIF_LOG(Error) << "[Mif::Net::Http::Session::OnClose] "
                                << "Failed to call OnClose handler. Error: " << e.what();
                        }
                        catch (...)
                        {
                            MIF_LOG(Error) << "[Mif::Net::Http::Session::OnClose] "
                                << "Failed to call OnClose handler. Error: unknown";
                        }
                    }

                    void OnResponse(ResponseType &response, boost::string_view target) noexcept
                    {
                        try
                        {
                            Detail::InputPack<ResponseType> pack{response, std::string{target}};

                            m_handler(pack);
                        }
                        catch (std::exception const &e)
                        {
                            MIF_LOG(Warning) << "[Mif::Net::Http::Session::OnResponse] "
                                << "Failed to process response. Error: " << e.what();
                        }
                        catch (...)
                        {
                            MIF_LOG(Warning) << "[Mif::Net::Http::Session::OnResponse] "
                                << "Failed to process response. Error: unknown";
                        }
                    }
                };

            }   // namespace

            class Connection::Impl final
            {
            public:
                Impl(Params const &params, ClientHandler const &handler,
                        OnCloseHandler const &onClose)
                    : m_handler{handler}
                    , m_onClose{onClose}
                    , m_params{params}
                {
                    try
                    {
                        boost::asio::ip::tcp::resolver resolver{m_ioc};
                        m_destinations = resolver.resolve(params.host, params.port);
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Fatal) << "[Mif::Net::Http::Connection::Impl] "
                                << "Failed to resolve address where host is \"" << params.host << "\" "
                                << "and port is \"" << params.port << "\". Error: " << e.what();
                        throw;
                    }

                    m_worker = boost::make_unique<std::thread>(
                            [this]
                            {
                                while (m_isRun)
                                {
                                    try
                                    {
                                        m_ioc.run();
                                    }
                                    catch (std::exception const &e)
                                    {
                                        MIF_LOG(Fatal) << "[Mif::Net::Http::Connection::Impl] "
                                                << "Failed to run ioc. Error: " << e.what();

                                        std::abort();
                                    }

                                    if (m_isRun)
                                    {
                                        MIF_LOG(Warning) << "[Mif::Net::Http::Connection::Impl] "
                                                << "Ioc has been interrupted and run again.";
                                        std::this_thread::sleep_for(std::chrono::milliseconds{20});;
                                    }
                                }
                            }
                        );
                }

                Impl(std::string const &host, std::string const &port,
                        ClientHandler const &handler, OnCloseHandler const &onClose)
                    : Impl{[&host, &port] { Params prm; prm.host = host; prm.port = port; return prm; } (), handler, onClose}
                {
                }

                ~Impl() noexcept
                {
                    m_isRun = false;

                    try
                    {
                        m_work.reset();
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Fatal) << "[Mif::Net::Http::Connection::~Impl] "
                                << "Failed to reset work guard. Error: " << e.what();

                        std::abort();
                    }

                    try
                    {
                        if (!m_ioc.stopped())
                            m_ioc.stop();
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Fatal) << "[Mif::Net::Http::Connection::~Impl] "
                                << "Failed to stop ioc. Error: " << e.what();

                        std::abort();
                    }

                    try
                    {
                        if (m_worker)
                            m_worker->join();
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Fatal) << "[Mif::Net::Http::Connection::~Impl] "
                                << "Failed to join worker thread. Error: " << e.what();

                        std::abort();
                    }
                }

                bool IsClosed() const
                {
                    return m_session ? m_session->IsClosed() : false;
                }

                IOutputPackPtr CreateRequest() const
                {
                    return std::make_shared<Detail::OutputPack<Session::Request>>(Session::Request{});
                }

                void MakeRequest(Method::Type method, std::string const &target, IOutputPackPtr pack)
                {
                    auto &request = dynamic_cast<Detail::OutputPack<Session::Request> &>(*pack).GetData();

                    request.target(target);
                    request.method(Detail::Utility::ConvertMethodType(method));

                    if (!m_session || m_session->IsClosed())
                    {
                        m_session = std::make_shared<Session>(m_ioc,
                                m_params, m_handler, m_onClose);
                        m_session->Run(m_destinations, m_params, std::move(pack));
                    }
                    else
                    {
                        m_session->Post(std::move(pack));
                    }
                }

            private:
                ClientHandler m_handler;
                OnCloseHandler m_onClose;
                Params m_params;

                std::atomic<bool> m_isRun{true};

                boost::asio::io_context m_ioc;

                using IocExecutorType = boost::asio::io_context::executor_type;
                using WorkGuardType = boost::asio::executor_work_guard<IocExecutorType>;

                WorkGuardType m_work{boost::asio::make_work_guard(m_ioc)};

                std::unique_ptr<std::thread> m_worker;

                boost::asio::ip::tcp::resolver::results_type m_destinations;

                std::shared_ptr<Session> m_session;
            };


            Connection::Connection(std::string const &host, std::string const &port,
                    ClientHandler const &handler, OnCloseHandler const &onClose)
                : m_impl{new Impl{host, port, handler, onClose}}
            {
            }

            Connection::Connection(Params const &params, ClientHandler const &handler,
                    OnCloseHandler const &onClose)
                : m_impl{new Impl{params, handler, onClose}}
            {
            }

            Connection::~Connection()
            {
            }

            Connection::IOutputPackPtr Connection::CreateRequest() const
            {
                if (!m_impl)
                    throw std::runtime_error{"[Mif::Net::Http::Connection::CreateRequest] Object was moved."};
                return m_impl->CreateRequest();
            }

            void Connection::MakeRequest(Method::Type method, std::string const &target, IOutputPackPtr pack)
            {
                if (!m_impl)
                    throw std::runtime_error{"[Mif::Net::Http::Connection::MakeRequest] Object was moved."};
                m_impl->MakeRequest(method, target, std::move(pack));
            }

            bool Connection::IsClosed() const
            {
                if (!m_impl)
                    throw std::runtime_error{"[Mif::Net::Http::Connection::IsClosed] Object was moved."};
                return m_impl->IsClosed();
            }

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
