//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_IWEB_SERVICE_H__
#define __MIF_NET_HTTP_IWEB_SERVICE_H__

// MIF
#include "mif/net/http/iinput_pack.h"
#include "mif/net/http/ioutput_pack.h"
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            struct IWebService
                : public Service::Inherit<Service::IService>
            {
                virtual void OnRequest(IInputPack const &request, IOutputPack &response) = 0;
            };

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_IWEB_SERVICE_H__
