//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_INPUT_PACK_H__
#define __MIF_NET_HTTP_DETAIL_INPUT_PACK_H__

// STD
#include <memory>

// EVENT
#include <event2/http.h>

// MIF
#include "mif/net/http/iinput_pack.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                class InputPack final
                    : public IInputPack
                {
                public:
                    InputPack(evhttp_request *request);

                private:
                    evhttp_request *m_request;
                    std::unique_ptr<evhttp_uri, decltype(&evhttp_uri_free)> m_uri{nullptr, &evhttp_uri_free};

                    // IInputPack
                    virtual Method::Type GetType() const override final;
                    virtual std::string GetHost() const override final;
                    virtual std::uint16_t GetPort() const override final;
                    virtual std::string GetSchema() const override final;
                    virtual std::string GetUserInfo() const override final;
                    virtual std::string GetPath() const override final;
                    virtual std::string GetQuery() const override final;
                    virtual std::string GetFragment() const override final;
                    virtual Params GetParams() const override final;
                    virtual Headers GetHeaders() const override final;
                    virtual Common::Buffer GetData() const override final;
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_INPUT_PACK_H__
