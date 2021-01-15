//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2018
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_RESULT_H__
#define __MIF_NET_HTTP_DETAIL_RESULT_H__

// STD
#include <string>
#include <utility>

// MIF
#include "mif/common/types.h"
#include "mif/net/http/detail/params.h"
#include "mif/net/http/constants.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                template <typename TSerializer>
                class Result final
                {
                public:
                    using Headers = Params<Tag::Headers>;

                    template <typename T>
                    Result(T const &data, std::string const &contentType = TSerializer::GetContentType())
                        : m_value{TSerializer::Serialize(data)}
                        , m_headers{{Constants::Header::Response::ContentType::Value, contentType}}
                    {
                    }

                    template <typename T>
                    Result(T const &data, Headers::Type const &headers,
                            std::string const &contentType = TSerializer::GetContentType())
                        : m_value{TSerializer::Serialize(data)}
                        , m_headers{headers}
                    {
                        if (m_headers.find(Constants::Header::Response::ContentType::Value) == std::end(m_headers))
                            m_headers.emplace(Constants::Header::Response::ContentType::Value, contentType);
                    }

                    template <typename TOther>
                    Result(Result<TOther> const &other)
                        : m_value{other.m_value}
                        , m_headers{other.m_headers}
                    {
                    }

                    Common::Buffer GetValue()
                    {
                        return std::move(m_value);
                    }

                    Headers const GetHeaders() const
                    {
                        return m_headers;
                    }

                private:
                    template <typename>
                    friend class Result;

                    Common::Buffer m_value;
                    Headers::Type m_headers;
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_RESULT_H__
