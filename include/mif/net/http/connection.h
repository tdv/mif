//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_CONNECTION_H__
#define __MIF_NET_HTTP_CONNECTION_H__

// STD
#include <memory>
#include <string>

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
                using IOutputPackPtr = std::unique_ptr<IOutputPack>;

                Connection(std::string const &host, std::string const &port,
                    ClientHandler const &handler, OnCloseHandler const &onClose = [] () {});

                ~Connection();

                IOutputPackPtr CreateRequest() const;
                void MakeRequest(Method::Type method, std::string const &request, IOutputPackPtr pack);
                bool IsClosed() const;

            private:
                class Impl;
                std::unique_ptr<Impl> m_impl;
            };

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_CONNECTION_H__
