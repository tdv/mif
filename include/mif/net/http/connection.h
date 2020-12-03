//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_CONNECTION_H__
#define __MIF_NET_HTTP_CONNECTION_H__

// STD
#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>

// BOOST
#include <boost/optional.hpp>

// MIF
#include "mif/net/http/methods.h"
#include "mif/net/http/request_handler.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            class Connection final
            {
            public:
                using OnCloseHandler = std::function<void ()>;
                using IOutputPackPtr = std::shared_ptr<IOutputPack>;

                struct Params
                {
                    std::string host;
                    std::string port;
                    boost::optional<std::chrono::seconds> connectionTimeout;
                    boost::optional<std::chrono::seconds> requestTimeout;
                    std::size_t retriesCount = std::numeric_limits<std::size_t>::max();
                    std::size_t maxHeaderSize = std::numeric_limits<std::size_t>::max();
                    std::size_t maxBodySize = std::numeric_limits<std::size_t>::max();
                };

                Connection(std::string const &host, std::string const &port,
                    ClientHandler const &handler, OnCloseHandler const &onClose = [] () {});

                Connection(Params const &params, ClientHandler const &handler,
                    OnCloseHandler const &onClose = [] () {});

                ~Connection();

                IOutputPackPtr CreateRequest() const;
                void MakeRequest(Method::Type method, std::string const &target, IOutputPackPtr pack);
                bool IsClosed() const;

            private:
                class Impl;
                std::unique_ptr<Impl> m_impl;
            };

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_CONNECTION_H__
