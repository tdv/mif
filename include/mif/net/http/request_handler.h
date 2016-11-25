//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_REQUEST_HANDLER_H__
#define __MIF_NET_HTTP_REQUEST_HANDLER_H__

// STD
#include <functional>

// MIF
#include "mif/net/http/irequest.h"
#include "mif/net/http/iresponse.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            using RequestHandler = std::function<void (IRequest const &, IResponse &)>;

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_REQUEST_HANDLER_H__
