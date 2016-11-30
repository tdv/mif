//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_RESPONSE_H__
#define __MIF_NET_HTTP_DETAIL_RESPONSE_H__

// EVENT
#include <event2/buffer.h>
#include <event2/http.h>

// MIF
#include "mif/net/http/iresponse.h"

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
                {
                public:
                    Response(evhttp_request *request);

                    void Send();

                private:
                    evhttp_request *m_request = nullptr;
                    evbuffer *m_responseBuffer = nullptr;
                    evkeyvalq *m_headers = nullptr;

                    Code m_code = Code::Ok;
                    std::string m_reason;
                    Common::Buffer m_buffer;

                    static void CleanUpData(void const *data, size_t datalen, void *extra);
                    int ConvertCode(Code code) const;

                    // IResponse
                    virtual void SetCode(Code code) override final;
                    virtual void SetReason(std::string const &reason) override final;

                    virtual void SetHeader(std::string const &key, std::string const &value) override final;
                    virtual void SetData(Common::Buffer buffer) override final;
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_RESPONSE_H__
