//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_SERVLET_H__
#define __MIF_NET_HTTP_SERVLET_H__

// MIF
#include "mif/net/iclient_factory.h"
#include "mif/net/http/request_handler.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            ServerHandler MakeServlet(std::shared_ptr<IClientFactory> factory, std::uint32_t sessionTimeout = 180 /*seconds*/);

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_SERVLET_H__
