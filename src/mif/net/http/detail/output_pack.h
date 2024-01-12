//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_OUTPUT_PACK_H__
#define __MIF_NET_HTTP_DETAIL_OUTPUT_PACK_H__

// STD
#include <memory>
#include <stdexcept>

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

                template <bool isRequest, typename TFields>
                class OutputPack<boost::beast::http::message<isRequest, boost::beast::http::buffer_body, TFields>> final
                    : public IOutputPack
                {
                public:
                    using MessageType = boost::beast::http::message<isRequest, boost::beast::http::buffer_body, TFields>;
                    using SerializerType = boost::beast::http::serializer<isRequest, boost::beast::http::buffer_body, TFields>;

                    explicit OutputPack(std::size_t chunkSize = 1024 * 1024)
                        : m_chunkSize{chunkSize}
                    {
                    }

                    explicit OutputPack(MessageType &&data, std::size_t chunkSize = 256 * 1024)
                        : m_data{std::move(data)}
                        , m_chunkSize{chunkSize}
                    {
                    }

                    ~OutputPack() = default;

                    MessageType& GetData() noexcept
                    {
                        if (!m_serializer)
                            SetData(Common::BufferPtr{});

                        return m_data;
                    }

                    SerializerType& GetNextPortion()
                    {
                        if (!m_serializer)
                        {
                            throw std::logic_error{"[Mif::Net::Http::Detail::OutputPack::GetNextPortion] "
                                                  "There is no any portion of the data."};
                        }

                        auto &body = m_data.body();

                        if (!body.more)
                        {
                            throw std::logic_error{"[Mif::Net::Http::Detail::OutputPack::GetNextPortion] "
                                                  "There is no more data."};
                        }

                        body.data = m_bufferPtr;
                        body.size = std::min<std::size_t>(m_buffer->size() -
                                (m_bufferPtr - m_buffer->data()), m_chunkSize);
                        body.more = (m_buffer->size() - (m_bufferPtr - m_buffer->data()))
                                > m_chunkSize;

                        m_bufferPtr += body.size;

                        return *m_serializer;
                    }

                private:
                    using SerializerPtr = std::unique_ptr<SerializerType>;

                    MessageType m_data;
                    std::size_t m_chunkSize;

                    SerializerPtr m_serializer;

                    Common::BufferPtr m_buffer;
                    char *m_bufferPtr = nullptr;

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

                    template <typename Y>
                    auto GetCode(Y const &data) const
                        -> decltype (Utility::ConvertCode(data.result))
                    {
                        return Utility::ConvertCode(data.result());
                    }

                    Code GetCode(...) const
                    {
                        throw std::logic_error{"[Mif::Net::Http::Detail::OutputPack::GetCode] "
                                "You can't get code for a request."};
                    }

                    // IOutputPack
                    virtual Code GetCode() const override final
                    {
                        return GetCode(m_data);
                    }

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
                        m_serializer.reset();

                        std::swap(m_buffer, buffer);

                        auto &body = m_data.body();

                        if (!m_buffer || m_buffer->empty())
                        {
                            body.data = nullptr;
                            body.size = 0;
                            body.more = false;
                        }
                        else if (m_buffer->size() < m_chunkSize)
                        {
                            body.data = m_buffer->data();
                            body.size = m_buffer->size();
                            body.more = false;
                        }
                        else
                        {
                            body.data = nullptr;
                            body.size = 0;
                            body.more = true;
                        }

                        m_serializer.reset(new SerializerType{m_data});

                        if (m_buffer && !m_buffer->empty())
                            m_bufferPtr = m_buffer->data();
                    }
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_OUTPUT_PACK_H__
