//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>
#include <utility>

// EVENT
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>

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

            class Connection::Impl final
            {
            public:
                Impl(Impl const &) = delete;
                Impl& operator = (Impl const &) = delete;
                Impl(Impl &&) = delete;
                Impl& operator = (Impl &&) = delete;

                Impl(std::string const &host, std::string const &port, ClientHandler const &handler)
                    : m_handler{handler}
                    , m_base{Detail::Utility::CreateEventBase()}
                    , m_connection{evhttp_connection_base_new(m_base.get(), nullptr, host.c_str(),
                        static_cast<ev_uint16_t>(std::stoi(port))), &evhttp_connection_free}
                {
                    if (!m_connection)
                    {
                        throw std::runtime_error{"[Mif::Net::Http::Connection::Impl] "
                            "Failed to create connection to \"" + host + ":" + port + "\"."};
                    }

                    evhttp_connection_set_closecb(m_connection.get(), &Impl::OnClose, this);
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
                    out->ReleaseNewRequest();
                    if (evhttp_make_request(m_connection.get(), out->GetRequest(),
                        static_cast<evhttp_cmd_type>(Detail::Utility::ConvertMethodType(method)),
                        request.c_str()))
                    {
                        throw std::runtime_error{"[Mif::Net::Http::Connection::Impl::MakeRequest] Failed to make request for \"" + request + "\""};
                    }
                }

                void Reconnect(std::string const &host, std::string const &port)
                {
                    {
                        ConnectionPtr connection{evhttp_connection_base_new(m_base.get(), nullptr, host.c_str(),
                            static_cast<ev_uint16_t>(std::stoi(port))), &evhttp_connection_free};
                        if (!connection)
                        {
                            throw std::runtime_error{"[Mif::Net::Http::Connection::Impl::Reconnect] "
                                "Failed to create connection to \"" + host + ":" + port + "\"."};
                        }

                        std::swap(connection, m_connection);
                    }

                    evhttp_connection_set_closecb(m_connection.get(), &Impl::OnClose, this);
                }

                void Run()
                {
                    auto code = event_base_loop(m_base.get(), 0);
                    if (code < 0)
                    {
                        MIF_LOG(Warning) << "[Mif::Net::Http::Connection::Impl::Run] "
                            << "Message loop was broken with code \"" << code << "\".";
                    }
                }

            private:
                using ConnectionPtr = std::unique_ptr<evhttp_connection, decltype(&evhttp_connection_free)>;

                ClientHandler m_handler;

                Detail::Utility::EventBasePtr m_base;
                ConnectionPtr m_connection;

                static void OnClose(evhttp_connection *connection, void *arg)
                {
                    // TODO:
                    (void)connection;
                    (void)arg;
                }

                static void OnRequestDone(evhttp_request *request, void *arg)
                {
                    if (!arg)
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Connection::Impl::OnRequestDone] "
                            << "No argument pointer.";
                        return;
                    }

                    if (!request || !evhttp_request_get_response_code(request))
                    {
                        MIF_LOG(Warning) << "[Mif::Net::Http::Connection::Impl::OnRequestDone] "
                            << "Connection refused.";
                        return;
                    }

                    reinterpret_cast<Impl *>(arg)->OnRequest(request);
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


            Connection::Connection(std::string const &host, std::string const &port, ClientHandler const &handler)
                : m_impl{new Impl{host, port, handler}}
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

            void Connection::Reconnect(std::string const &host, std::string const &port)
            {
                if (!m_impl)
                    throw std::runtime_error{"[Mif::Net::Http::Connection::Reconnect] Object was moved."};
                m_impl->Reconnect(host, port);
            }

            void Connection::Run()
            {
                if (!m_impl)
                    throw std::runtime_error{"[Mif::Net::Http::Connection::Run] Object was moved."};
                m_impl->Run();
            }

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
