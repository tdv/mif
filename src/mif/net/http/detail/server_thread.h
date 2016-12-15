//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_SERVER_THREAD_H__
#define __MIF_NET_HTTP_DETAIL_SERVER_THREAD_H__

// STD
#include <chrono>
#include <stdexcept>
#include <memory>
#include <string>
#include <thread>

// EVENT
#include <event2/util.h>

// MIF
#include "mif/net/http/request_handler.h"

// THIS
#include "server.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                class ServerThread final
                {
                public:
                    ServerThread(std::string const &host, std::string const &port, ServerHandler const &handler,
                        Methods const &allowedMethods, std::size_t headersSize, std::size_t bodySize);
                    ServerThread(evutil_socket_t socket, ServerHandler const &handler,
                        Methods const &allowedMethods, std::size_t headersSize, std::size_t bodySize);

                    ~ServerThread();

                    evutil_socket_t GetSocket() const;

                private:
                    std::chrono::milliseconds const m_timeout{5};
                    std::exception_ptr m_exception;
                    std::unique_ptr<Server> m_server;
                    using ThreadPtr = std::unique_ptr<std::thread, void (*)(std::thread *)>;
                    ThreadPtr m_thread{nullptr, &ServerThread::ThreadDeleter};

                    static void ThreadDeleter(std::thread *thread);
                };

            } // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_SERVER_THREAD_H__
