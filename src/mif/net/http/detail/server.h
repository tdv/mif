//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_SERVER_H__
#define __MIF_NET_HTTP_DETAIL_SERVER_H__

// STD
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

// EVENT
#include <event2/event.h>
#include <event2/http.h>

// MIF
#include "mif/net/http/request_handler.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                class Server final
                {
                public:
                    Server(Server const &) = delete;
                    Server& operator = (Server const &) = delete;
                    Server(Server &&) = delete;
                    Server& operator = (Server &&) = delete;

                    Server(std::string const &host, std::string const &port, ServerHandler const &handler);
                    Server(evutil_socket_t socket, ServerHandler const &handler);

                    ~Server() noexcept;

                    evutil_socket_t GetSocket() const;
                    void Run();
                    void Stop();

                private:
                    ServerHandler m_handler;
                    evutil_socket_t m_socket = -1;

                    using EventPtr = std::unique_ptr<event, decltype(&event_free)>;
                    using EventBasePtr = std::unique_ptr<event_base, decltype(&event_base_free)>;
                    using HttpPtr = std::unique_ptr<evhttp, decltype(&evhttp_free)>;

                    std::uint32_t const m_timerPeriod = 200000;

                    std::atomic<bool> m_isActive{true};
                    std::atomic<bool> m_isRun{false};

                    EventBasePtr m_base{nullptr, &event_base_free};
                    EventPtr m_timer{nullptr, &event_free};
                    HttpPtr m_http{nullptr, &evhttp_free};

                    Server(ServerHandler const &handler);

                    static void OnTimer(evutil_socket_t, short, void *arg);
                    static void OnRequest(evhttp_request *req, void *arg);
                    void OnRequest(evhttp_request *req);
                };

            } // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_SERVER_H__
