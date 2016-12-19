//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <vector>

// MIF
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
                Impl(std::string const &host, std::string const &port, std::uint16_t workers,
                     ServerHandler const &handler, Methods const &allowedMethods,
                     std::size_t headersSize, std::size_t bodySize, std::size_t requestTimeout)
                {
                    Detail::LibEventInitializer::Init();

                    m_items.reserve(workers);

                    evutil_socket_t socket = -1;

                    while (workers--)
                    {
                        if (socket == -1)
                        {
                            ItemPtr item{new Detail::ServerThread{host, port, handler,
                                allowedMethods, headersSize, bodySize, requestTimeout}};
                            socket = item->GetSocket();
                            m_items.push_back(std::move(item));
                        }
                        else
                        {
                            m_items.push_back(std::move(ItemPtr{new Detail::ServerThread{socket, handler,
                                allowedMethods, headersSize, bodySize, requestTimeout}}));
                        }
                    }
                }

            private:
                using ItemPtr = std::unique_ptr<Detail::ServerThread>;
                using Items = std::vector<ItemPtr>;

                Items m_items;
            };


            Server::Server(std::string const &host, std::string const &port, std::uint16_t workers,
                Methods const &allowedMethods, ServerHandler const &handler,
                std::size_t headersSize, std::size_t bodySize, std::size_t requestTimeout)
                : m_impl{new Impl{host, port, workers, handler, allowedMethods, headersSize, bodySize, requestTimeout}}
            {
            }

            Server::~Server()
            {
            }

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
