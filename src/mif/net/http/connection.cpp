//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// STD
#include <atomic>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

// EVENT
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>

// MIF
#include "mif/common/log.h"
#include "mif/common/unused.h"
#include "mif/net/http/connection.h"

// THIS
#include "detail/input_pack.h"
#include "detail/lib_event_initializer.h"
#include "detail/output_pack.h"
#include "detail/utility.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            class Connection::Impl final
            {
            public:
                Impl(Impl const &) = delete;
                Impl& operator = (Impl const &) = delete;
                Impl(Impl &&) = delete;
                Impl& operator = (Impl &&) = delete;

                Impl(Params const &params, ClientHandler const &handler,
                        OnCloseHandler const &onClose)
                    : m_handler{handler}
                    , m_onClose{onClose}
                    , m_host{params.host}
                    , m_port{params.port}
                    , m_base{Detail::Utility::CreateEventBase()}
                {
                    Detail::LibEventInitializer::Init();

                    m_connection.reset(evhttp_connection_base_new(m_base.get(), nullptr, params.host.c_str(),
                        static_cast<ev_uint16_t>(std::stoi(params.port))));

                    if (!m_connection)
                    {
                        throw std::runtime_error{"[Mif::Net::Http::Connection::Impl] "
                            "Failed to create connection to \"" + m_host + ":" + m_port + "\"."};
                    }

                    evhttp_connection_set_closecb(m_connection.get(), &Impl::OnClose, this);

                    if (params.timeout != std::chrono::seconds::max())
                        evhttp_connection_set_timeout(m_connection.get(), params.timeout.count());

                    if (params.retriesCount != std::numeric_limits<std::size_t>::max())
                        evhttp_connection_set_retries(m_connection.get(), params.retriesCount);

                    if (params.maxHeaderSize != std::numeric_limits<std::size_t>::max())
                        evhttp_connection_set_max_headers_size(m_connection.get(), params.maxHeaderSize);

                    if (params.maxBodySize != std::numeric_limits<std::size_t>::max())
                        evhttp_connection_set_max_body_size(m_connection.get(), params.maxBodySize);

                    {
                        EventPtr timer{event_new(m_base.get(), -1, EV_PERSIST, &Impl::OnTimer, this), &event_free};
                        if (!timer)
                            throw std::runtime_error{"[Mif::Net::Http::Connection::Impl] Failed to create timer object."};
                        timeval interval{0, m_period};
                        if (event_add(timer.get(), &interval))
                            throw std::runtime_error{"[Mif::Net::Http::Connection::Impl] Failed to initialize timer."};
                        std::swap(timer, m_timer);
                    }

                    m_thread.reset(new std::thread{std::bind(&Impl::Run, this)});
                }

                Impl(std::string const &host, std::string const &port,
                        ClientHandler const &handler, OnCloseHandler const &onClose)
                    : Impl{[&host, &port] { Params prm; prm.host = host; prm.port = port; return prm; } (), handler, onClose}
                {
                }

                ~Impl()
                {
                    if (event_del(m_timer.get()))
                        MIF_LOG(Warning) << "[Mif::Net::Http::Connection::~Impl] Failed to destroy timer.";
                    if (!m_isClosed && event_base_loopbreak(m_base.get()))
                    {
                        MIF_LOG(Warning) << "[Mif::Net::Http::Connection::~Impl] "
                            << "Failed to post 'stop' to server.";
                    }
                    else
                    {
                        m_isClosed = true;
                    }

                    m_thread.reset();
                }

                bool IsClosed() const
                {
                    return m_isClosed;
                }

                IOutputPackPtr CreateRequest() const
                {
                    Detail::OutputPack::RequestPtr request{evhttp_request_new(&Impl::OnRequestDone,
                        const_cast<Impl *>(this)), &evhttp_request_free};
                    if (!request)
                        throw std::runtime_error{"[Mif::Net::Http::Connection::Impl::CreateRequest] Failed to create request."};

                    IOutputPackPtr pack{new Detail::OutputPack{std::move(request)}};
                    return std::move(pack);
                }

                void MakeRequest(Method::Type method, std::string const &request, IOutputPackPtr pack)
                {
                    if (!pack)
                        throw std::invalid_argument{"[Mif::Net::Http::Connection::Impl::MakeRequest] Empty package for \"" + request + "\""};
                    auto *out = static_cast<Detail::OutputPack *>(pack.get());
                    out->MoveDataToBuffer();
                    out->ReleaseNewRequest();
                    if (evhttp_make_request(m_connection.get(), out->GetRequest(),
                        static_cast<evhttp_cmd_type>(Detail::Utility::ConvertMethodType(method)),
                        request.c_str()))
                    {
                        throw std::runtime_error{"[Mif::Net::Http::Connection::Impl::MakeRequest] Failed to make request for \"" + request + "\""};
                    }

                }

            private:
                using ConnectionPtr = std::unique_ptr<evhttp_connection, decltype(&evhttp_connection_free)>;

                ClientHandler m_handler;
                OnCloseHandler m_onClose;
                std::string m_host;
                std::string m_port;

                Detail::Utility::EventBasePtr m_base;

                std::uint32_t const m_period = 500;
                using EventPtr = std::unique_ptr<event, decltype(&event_free)>;
                EventPtr m_timer{nullptr, &event_free};

                ConnectionPtr m_connection{nullptr, &evhttp_connection_free};

                std::atomic<bool> m_isClosed{false};
                using ThreadPtr = std::unique_ptr<std::thread, std::function<void (std::thread *)>>;
                ThreadPtr m_thread{nullptr, std::bind(&Impl::ThreadDeleter, this, std::placeholders::_1)};

                static void OnTimer(evutil_socket_t, short, void *)
                {
                }

                void ThreadDeleter(std::thread *t)
                {
                    try
                    {
                        if (t)
                            t->join();
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Warning) << "[Mif::Net::Http::Connection::Impl::ThreadDeleter] "
                            << "Failed to join thread. Error: " << e.what();
                    }
                    catch (...)
                    {
                        MIF_LOG(Warning) << "[Mif::Net::Http::Connection::Impl::ThreadDeleter] "
                            << "Failed to join thread. Error: unknown";
                    }
                    delete t;
                }

                void Run()
                {
                    auto code = event_base_loop(m_base.get(), 0);
                    if (code < 0)
                    {
                        MIF_LOG(Warning) << "[Mif::Net::Http::Connection::Impl::Run] "
                            << "Message loop was broken with code \"" << code << "\".";
                    }

                    m_isClosed = true;
                }

                void OnClose()
                {
                    try
                    {
                        m_isClosed = true;
                        m_onClose();
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Connection::Impl::OnClose] "
                            << "Failed to call OnClose handler. Error: " << e.what();
                    }
                    catch (...)
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Connection::Impl::OnClose] "
                            << "Failed to call OnClose handler. Error: unknown";
                    }
                }

                static void OnClose(evhttp_connection *, void *arg)
                {
                    if (!arg)
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Connection::Impl::OnClose] "
                            << "No argument pointer.";
                        return;
                    }

                    auto *self = reinterpret_cast<Impl *>(arg);
                    self->OnClose();
                }

                static void OnRequestDone(evhttp_request *request, void *arg)
                {
                    if (!arg)
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Connection::Impl::OnRequestDone] "
                            << "No argument pointer.";
                        return;
                    }

                    auto *self = reinterpret_cast<Impl *>(arg);

                    if (!request || !evhttp_request_get_response_code(request) || !request->response_code)
                    {
                        self->OnClose();

                        MIF_LOG(Warning) << "[Mif::Net::Http::Connection::Impl::OnRequestDone] "
                            << "[" << self->m_host << ":" << self->m_port << "] Connection refused.";

                        return;
                    }

                    self->OnRequest(request);
                }

                void OnRequest(evhttp_request *request)
                {
                    try
                    {
                        Detail::InputPack pack{request};
                        m_handler(pack);
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Warning) << "[Mif::Net::Http::Connection::Impl::OnRequest] "
                            << "Failed to process request. Error: " << e.what();
                    }
                    catch (...)
                    {
                        MIF_LOG(Warning) << "[Mif::Net::Http::Connection::Impl::OnRequest] "
                            << "Failed to process request. Error: unknown";
                    }
                }
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

            void Connection::MakeRequest(Method::Type method, std::string const &request, IOutputPackPtr pack)
            {
                if (!m_impl)
                    throw std::runtime_error{"[Mif::Net::Http::Connection::MakeRequest] Object was moved."};
                m_impl->MakeRequest(method, request, std::move(pack));
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
