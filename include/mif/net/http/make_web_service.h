//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_MAKE_WEB_SERVICE_H__
#define __MIF_NET_HTTP_MAKE_WEB_SERVICE_H__

// STD
#include <functional>
#include <type_traits>
#include <utility>

// MIF
#include "mif/net/http/request_handler.h"
#include "mif/net/http/iweb_service.h"
#include "mif/service/create.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            inline ServerHandler MakeWebService(IWebServicePtr service)
            {
                auto handler = std::bind(&IWebService::OnRequest, service, std::placeholders::_1, std::placeholders::_2);
                return handler;
            }

            template <typename TService, typename ... TArgs>
            inline typename std::enable_if<std::is_base_of<IWebService, TService>::value, ServerHandler>::type
            MakeWebService(TArgs && ... args)
            {
                auto service = Service::Make<TService, IWebService>(std::forward<TArgs>(args) ... );
                auto handler = std::bind(&IWebService::OnRequest, service, std::placeholders::_1, std::placeholders::_2);
                return handler;
            }

            template <Service::ServiceId Id, typename ... TArgs>
            inline ServerHandler MakeWebService(TArgs && ... args)
            {
                auto service = Service::Create<Id, IWebService>(std::forward<TArgs>(args) ... );
                auto handler = std::bind(&IWebService::OnRequest, service, std::placeholders::_1, std::placeholders::_2);
                return handler;
            }

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif


#endif  // !__MIF_NET_HTTP_MAKE_WEB_SERVICE_H__
