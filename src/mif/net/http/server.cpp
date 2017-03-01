//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <functional>
#include <vector>

// MIF
#include "mif/common/thread_pool.h"
#include "mif/net/http/server.h"

// THIS
#include "detail/server_thread.h"
#include "detail/lib_event_initializer.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            class Server::Impl final
            {
            public:
                Impl(std::string const &host, std::string const &port,
                        std::uint16_t httpThreads, std::uint16_t workerThreads,
                        ServerHandlers const &handlers, Methods const &allowedMethods,
                        std::size_t headersSize, std::size_t bodySize, std::size_t requestTimeout)
                    : m_handlers{handlers}
                {
                    auto handler = std::bind(&Impl::OnRequest, this, std::placeholders::_1, std::placeholders::_2);

                    auto workers = Common::CreateThreadPool(workerThreads);

                    Detail::LibEventInitializer::Init();

                    m_items.reserve(httpThreads);

                    evutil_socket_t socket = -1;

                    while (httpThreads--)
                    {
                        if (socket == -1)
                        {
                            ItemPtr item{new Detail::ServerThread{host, port, workers,
                                handler, allowedMethods, headersSize, bodySize, requestTimeout}};
                            socket = item->GetSocket();
                            m_items.push_back(std::move(item));
                        }
                        else
                        {
                            m_items.push_back(std::move(ItemPtr{new Detail::ServerThread{socket, workers,
                                handler, allowedMethods, headersSize, bodySize, requestTimeout}}));
                        }
                    }
                }

            private:
                using ItemPtr = std::unique_ptr<Detail::ServerThread>;
                using Items = std::vector<ItemPtr>;

                ServerHandlers m_handlers;

                Items m_items;

                void OnRequest(IInputPack const &in, IOutputPack &out)
                {
                    for (std::string path = in.GetPath() ; ; )
                    {
                        auto iter = m_handlers.find(path);
                        if (iter != std::end(m_handlers))
                        {
                            iter->second(in, out);
                            return;
                        }

                        if (path.length() == 1 && path == "/")
                            break;

                        auto const pos = path.find_last_of('/');
                        if (pos == std::string::npos)
                            break;
                        path = path.substr(0, pos);
                        if (path.empty())
                            path = "/";
                    }

                    throw std::runtime_error{"[Mif::Net::Http::Server::Impl] Failed to process request. "
                            "Handler for resource \"" + in.GetPath() + "\" not found."};
                }
            };


            Server::Server(std::string const &host, std::string const &port,
                std::uint16_t httpThreads, std::uint16_t workerThreads,
                Methods const &allowedMethods, ServerHandlers const &handlers,
                std::size_t headersSize, std::size_t bodySize, std::size_t requestTimeout)
                : m_impl{new Impl{host, port, httpThreads, workerThreads, handlers, allowedMethods, headersSize, bodySize, requestTimeout}}
            {
            }

            Server::~Server()
            {
            }

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
