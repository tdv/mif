//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_OUTPUT_PACK_H__
#define __MIF_NET_HTTP_DETAIL_OUTPUT_PACK_H__

// STD
#include <memory>

// EVENT
#include <event2/buffer.h>
#include <event2/http.h>

// MIF
#include "mif/net/http/ioutput_pack.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                class OutputPack final
                    : public IOutputPack
                {
                public:
                    using RequestPtr = std::unique_ptr<evhttp_request, decltype(&evhttp_request_free)>;

                    explicit OutputPack(evhttp_request *request);
                    explicit OutputPack(RequestPtr request);

                    void Send();
                    evhttp_request* GetRequest();
                    void MoveDataToBuffer();
                    void ReleaseNewRequest();

                private:
                    RequestPtr m_newRequest{nullptr, &evhttp_request_free};

                    evhttp_request *m_request = nullptr;
                    evbuffer *m_responseBuffer = nullptr;
                    evkeyvalq *m_headers = nullptr;

                    Code m_code = Code::Ok;
                    std::string m_reason;
                    Common::Buffer m_buffer;

                    static void CleanUpData(void const *data, size_t datalen, void *extra);

                    // IOutputPack
                    virtual void SetCode(Code code) override final;
                    virtual void SetReason(std::string const &reason) override final;

                    virtual void SetHeader(std::string const &key, std::string const &value) override final;
                    virtual void SetData(Common::Buffer buffer) override final;
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_OUTPUT_PACK_H__
