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
#include <stdexcept>

// BOOST
#include <boost/beast/http.hpp>
#include <boost/optional.hpp>

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

                template <typename T>
                class OutputPack final
                    : public IOutputPack
                {
                public:
                    explicit OutputPack(T &data)
                        : m_data{data}
                    {
                    }

                    explicit OutputPack(T &&data)
                        : m_holder{std::move(data)}
                        , m_data{m_holder.get()}
                    {
                    }

                    T& GetData() noexcept
                    {
                        return m_data;
                    }

                    T const & GetData() const noexcept
                    {
                        return m_data;
                    }

                private:
                    boost::optional<T> m_holder;
                    T &m_data;
                    Common::BufferPtr m_buffer;

                    template <typename Y>
                    auto SetReason(Y &data, std::string const &reason) const
                        -> decltype (data.reason(reason))
                    {
                        return data.reason(reason);
                    }

                    void SetReason(...) const
                    {
                        throw std::logic_error{"[Mif::Net::Http::Detail::OutputPack::SetReason] "
                                "You can't set a reason for a request."};
                    }

                    template <typename Y>
                    auto SetCode(Y &data, Code code) const
                        -> decltype (data.result(Utility::ConvertCode(code)))
                    {
                        return data.result(Utility::ConvertCode(code));
                    }

                    void SetCode(...) const
                    {
                        throw std::logic_error{"[Mif::Net::Http::Detail::OutputPack::SetCode] "
                                "You can't set code for a request."};
                    }

                    // IOutputPack
                    virtual void SetCode(Code code) override final
                    {
                        SetCode(m_data, code);
                    }

                    virtual void SetReason(std::string const &reason) override final
                    {
                        SetReason(m_data, reason);
                    }

                    virtual void SetHeader(std::string const &key, std::string const &value) override final
                    {
                        m_data.set(key, value);
                    }

                    virtual void SetData(Common::Buffer buffer) override final
                    {
                        SetData(std::make_shared<Common::Buffer>(std::move(buffer)));
                    }

                    virtual void SetData(Common::BufferPtr buffer) override final
                    {
                        std::swap(m_buffer, buffer);

                        auto &body = m_data.body();
                        body.more = false;

                        if (m_buffer && !m_buffer->empty())
                        {
                            body.data = m_buffer->data();
                            body.size = m_buffer->size();
                        }
                        else
                        {
                            body.data = nullptr;
                            body.size = 0;
                        }

                        m_data.content_length(body.size);
                    }
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_OUTPUT_PACK_H__
