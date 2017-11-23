//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <memory>
#include <thread>

// MIF
#include "mif/common/log.h"
#include "mif/net/http/constants.h"

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

                Server::Server(ServerHandler const &handler, Methods const &allowedMethods, std::size_t headersSize,
                        std::size_t bodySize, std::size_t requestTimeout)
                    : m_handler{handler}
                    , m_base{Utility::CreateEventBase()}
                {
                    {
                        HttpPtr http{evhttp_new(m_base.get()), &evhttp_free};
                        if (!http)
                            throw std::runtime_error{"[Mif::Net::Http::Detail::Server] Failed to create http object."};

                        auto const methods = Utility::ConvertAllowedMethods(allowedMethods);
                        evhttp_set_allowed_methods(http.get(), methods);
                        evhttp_set_gencb(http.get(), &Server::OnRequest, this);

                        if (headersSize != static_cast<decltype(headersSize)>(-1))
                            evhttp_set_max_headers_size(http.get(), headersSize);
                        if (bodySize != static_cast<decltype(bodySize)>(-1))
                            evhttp_set_max_body_size(http.get(), bodySize);
                        if (requestTimeout != static_cast<decltype(requestTimeout)>(-1))
                            evhttp_set_timeout(http.get(), requestTimeout);

                        std::swap(http, m_http);
                    }
                }

                Server::Server(std::string const &host, std::string const &port, ServerHandler const &handler, Methods const &allowedMethods,
                        std::size_t headersSize, std::size_t bodySize, std::size_t requestTimeout)
                    : Server{handler, allowedMethods, headersSize, bodySize, requestTimeout}
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

                Server::Server(evutil_socket_t socket, ServerHandler const &handler, Methods const &allowedMethods,
                        std::size_t headersSize, std::size_t bodySize, std::size_t requestTimeout)
                    : Server{handler, allowedMethods, headersSize, bodySize, requestTimeout}
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

                    if (event_base_loopbreak(m_base.get()))
                    {
                        throw std::runtime_error{"[Mif::Net::Http::Detail::Server::Stop] "
                            "Failed to post 'stop' to server."};
                    }

                    while (m_isRun)
                        std::this_thread::sleep_for(std::chrono::microseconds{m_waitPeriod});
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
                        auto in = std::make_shared<InputPack >(req);
                        auto out = std::make_shared<OutputPack>(req);
                        ServerHandler handler{m_handler};
                        try
                        {
                            handler(*in, *out);
                            out->Send();
                        }
                        catch (std::exception const &e)
                        {
                            MIF_LOG(Warning) << "[Mif::Net::Http::Detail::Server] "
                                    << "Failed to process request. Error: " << e.what();
                            try
                            {
                                IOutputPack &pack = *out;
                                pack.SetHeader(Constants::Header::Response::Connection::Value,
                                        Constants::Value::Connection::Close::Value);
                                pack.SetCode(Code::BadMethod);
                                out->Send();
                            }
                            catch (std::exception const &ex)
                            {
                                MIF_LOG(Warning) << "[Mif::Net::Http::Detail::Server] "
                                        << "Failed to close connection. Error: " << ex.what();
                            }
                        }
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
