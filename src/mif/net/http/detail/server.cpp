//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <thread>

// MIF
#include "mif/common/log.h"

// THIS
#include "input_pack.h"
#include "output_pack.h"
#include "server.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                Server::Server(ServerHandler const &handler)
                    : m_handler{handler}
                {
                    {
                        using ConfigPtr = std::unique_ptr<event_config, decltype(&event_config_free)>;
                        ConfigPtr config{event_config_new(), &event_config_free};
                        if (!config)
                            throw std::runtime_error{"[Mif::Net::Http::Detail::Server] Failed to create configuration for creating event object."};

                        if (event_config_set_flag(config.get(),
                                EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST |
                                EVENT_BASE_FLAG_NO_CACHE_TIME |
                                EVENT_BASE_FLAG_IGNORE_ENV)
                            )
                        {
                            throw std::runtime_error{"[Mif::Net::Http::Detail::Server] Failed to set base options."};
                        }

                        EventBasePtr base{event_base_new_with_config(config.get()), &event_base_free};
                        if (!base)
                            throw std::runtime_error{"[Mif::Net::Http::Detail::Server] Failed to create base object."};
                        std::swap(base, m_base);
                    }

                    {
                        HttpPtr http{evhttp_new(m_base.get()), &evhttp_free};
                        if (!http)
                            throw std::runtime_error{"[Mif::Net::Http::Detail::Server] Failed to create http object."};

                        // TODO: get methods from params
                        evhttp_set_allowed_methods(http.get(), EVHTTP_REQ_GET | EVHTTP_REQ_POST);
                        evhttp_set_gencb(http.get(), &Server::OnRequest, this);

                        std::swap(http, m_http);
                    }

                    {
                        EventPtr timer{event_new(m_base.get(), -1, EV_PERSIST, &Server::OnTimer, this), &event_free};
                        if (!timer)
                            throw std::runtime_error{"[Mif::Net::Http::Detail::Server] Failed to create timer object."};
                        timeval interval{0, m_timerPeriod};
                        if (event_add(timer.get(), &interval))
                            throw std::runtime_error{"[Mif::Net::Http::Detail::Server] Failed to initialize timer."};
                        std::swap(timer, m_timer);
                    }
                }

                Server::Server(std::string const &host, std::string const &port, ServerHandler const &handler)
                    : Server{handler}
                {
                    if (host.empty())
                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Server] Host must not be empty."};
                    if (port.empty())
                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Server] Port must not be empty."};

                    auto *info = evhttp_bind_socket_with_handle(m_http.get(), host.c_str(), static_cast<ev_uint16_t>(std::stoi(port)));
                    if (!info)
                        throw std::runtime_error{"[Mif::Net::Http::Detail::Server] Failed to bind http server socket on host \"" + host + "\" and port \"" + port + "\"."};

                    m_socket = evhttp_bound_socket_get_fd(info);
                }

                Server::Server(evutil_socket_t socket, ServerHandler const &handler)
                    : Server{handler}
                {
                    if (socket == -1)
                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Server] Invalid input socket."};

                    if (evhttp_accept_socket(m_http.get(), socket))
                        throw std::runtime_error{"[Mif::Net::Http::Detail::Server] Failed to bind http server socket with already existing socket \"" + std::to_string(socket) + "\"."};

                    m_socket = socket;
                }

                Server::~Server() noexcept
                {
                    try
                    {
                        Stop();
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Detail::Server] Failed to stop server item. Error: " << e.what();
                    }
                    catch (...)
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Detail::Server] Failed to stop server item. Error: unknown.";
                    }
                }

                evutil_socket_t Server::GetSocket() const
                {
                    return m_socket;
                }

                void Server::Run()
                {
                    if (m_isRun)
                        return;
                    m_isActive = true;

                    m_isRun = true;

                    while (m_isActive)
                    {
                        auto code = event_base_loop(m_base.get(), 0);
                        if (code)
                        {
                            MIF_LOG(Warning) << "[Mif::Net::Http::Detail::Server::Run] "
                                << "Message loop was temporary broken with code \"" << code << "\".";
                        }
                    }

                    m_isRun = false;
                }

                void Server::Stop()
                {
                    if (!m_isActive)
                        return;

                    m_isActive = false;

                    while (m_isRun)
                        std::this_thread::sleep_for(std::chrono::microseconds{m_timerPeriod});
                }

                void Server::OnTimer(evutil_socket_t, short, void *arg)
                {
                    if (!arg)
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Detail::Server::OnTimer] "
                            << "Failed to process timer event. No arguments.";
                        return;
                    }

                    auto *self = reinterpret_cast<Server*>(arg);
                    if (self->m_isActive)
                        return;

                    if (event_base_got_break(self->m_base.get()))
                        return;

                    if (event_base_loopbreak(self->m_base.get()))
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Detail::Server::OnTimer] "
                            << "Failed to post 'stop' to server.";
                    }
                }

                void Server::OnRequest(evhttp_request *req, void *arg)
                {
                    if (!arg)
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Detail::Server::OnRequest] "
                            << "Failed to process request. No arguments.";
                        return;
                    }

                    if (!req)
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Detail::Server::OnRequest] "
                            << "Failed to process request. Empty request data.";
                        return;
                    }

                    auto *self = reinterpret_cast<Server*>(arg);

                    if (event_base_got_break(self->m_base.get()))
                    {
                        MIF_LOG(Warning) << "[Mif::Net::Http::Detail::Server::OnRequest] "
                            << "Message loop was stopped. The request will not be processed.";
                        return;
                    }

                    self->OnRequest(req);
                }

                void Server::OnRequest(evhttp_request *req)
                {
                    try
                    {
                        InputPack in{req};
                        OutputPack out{req};
                        m_handler(in, out);
                        out.Send();
                    }
                    catch (std::invalid_argument const &e)
                    {
                        std::string reason = "Bad request. ";
                        reason += e.what();
                        evhttp_send_error(req, HTTP_BADREQUEST, reason.c_str());
                    }
                    catch (std::exception const &e)
                    {
                        std::string reason = "Internal server error. ";
                        reason += e.what();
                        evhttp_send_error(req, HTTP_INTERNAL, reason.c_str());
                    }
                }

            } // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
