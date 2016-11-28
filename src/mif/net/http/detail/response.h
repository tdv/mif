//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_RESPONSE_H__
#define __MIF_NET_HTTP_DETAIL_RESPONSE_H__

// EVENT
#include <event2/http.h>

// MIF
#include "mif/net/http/iresponse.h"

// THIS
#include "isender.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                class Response final
                    : public IResponse
                    , public ISender
                {
                public:
                    Response(evhttp_request *request);

                private:
                    evhttp_request *m_request;

                    // IResponse

                    // ISender
                    virtual void Send() override final;
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_RESPONSE_H__
