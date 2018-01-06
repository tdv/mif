//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_CONTENT_H__
#define __MIF_NET_HTTP_DETAIL_CONTENT_H__

// STD
#include <memory>
#include <stdexcept>

// MIF
#include "mif/common/types.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                template <typename T, typename TConverter>
                class Content final
                {
                public:
                    using ContentType = Content<T, TConverter>;
                    using Type = T;
                    using Converter = TConverter;

                    Content() = default;
                    Content(Content &&) = default;
                    Content(Content const &) = delete;

                    Content(Common::Buffer const &value)
                        : m_value{new Type{Converter::template Convert<Type>(value)}}
                    {
                    }

                    explicit operator bool() const noexcept
                    {
                        return !!m_value;
                    }

                    Type const& Get() const
                    {
                        if (!*this)
                            throw std::runtime_error{"[Mif::Net::Http::WebService::Content] No content."};
                        return *m_value;
                    }

                private:
                    std::unique_ptr<T> const m_value;
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_CONTENT_H__
