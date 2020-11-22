//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_OUTPUT_PACK_H__
#define __MIF_NET_HTTP_DETAIL_OUTPUT_PACK_H__

// STD
#include <memory>

// BOOST
#include <boost/beast/http.hpp>

// MIF
#include "mif/net/http/ioutput_pack.h"

// THIS
#include "utility.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                template <typename>
                class OutputPack;

                template <typename TBody, typename TFields>
                class OutputPack<boost::beast::http::response<TBody, TFields>> final
                    : public IOutputPack
                {
                public:
                    using Response = boost::beast::http::response<TBody, TFields>;

                    OutputPack(Response &response)
                        : m_response{response}
                    {
                    }

                private:
                    Response &m_response;

                    // IOutputPack
                    virtual void SetCode(Code code) override final
                    {
                        m_response.result(Utility::ConvertCode(code));
                    }

                    virtual void SetReason(std::string const &reason) override final
                    {
                        m_response.reason(reason);
                    }

                    virtual void SetHeader(std::string const &key, std::string const &value) override final
                    {
                        m_response.set(key, value);
                    }

                    virtual void SetData(Common::Buffer buffer) override final
                    {
                        m_response.body() = buffer;
                    }

                    virtual void SetData(Common::BufferPtr buffer) override final
                    {
                        (void)buffer;
                    }
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_OUTPUT_PACK_H__
