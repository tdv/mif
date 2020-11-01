//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// MIF
#include "mif/common/unused.h"

// THIS
#include "output_pack.h"
#include "utility.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                OutputPack::OutputPack(evhttp_request *request)
                    : m_request{request}
                {
                    if (!m_request)
                        throw std::invalid_argument{"[Mif::Net::Http::Detail::OutputPack] Empty request pointer."};

                    if (!(m_responseBuffer = evhttp_request_get_output_buffer(m_request)))
                        throw std::runtime_error{"[Mif::Net::Http::Detail::OutputPack] Failed to get output buffer."};

                    if (!(m_headers = evhttp_request_get_output_headers(m_request)))
                        throw std::runtime_error{"[Mif::Net::Http::Detail::OutputPack] Failed to get output headers."};
                }

                OutputPack::OutputPack(RequestPtr request)
                    : OutputPack{request.get()}
                {
                    m_newRequest = std::move(request);
                }


                void OutputPack::Send()
                {
                    MoveDataToBuffer();
                    auto const code = Utility::ConvertCode(m_code);
                    evhttp_send_reply(m_request, code, m_reason.empty() ? Utility::GetReasonString(m_code) : m_reason.c_str(), m_responseBuffer);
                }

                evhttp_request* OutputPack::GetRequest()
                {
                    return m_request;
                }

                void OutputPack::MoveDataToBuffer()
                {
                    if (!m_buffer)
                        return;

                    std::unique_ptr<Common::BufferPtr> buf{new Common::BufferPtr{m_buffer}};
                    if (evbuffer_add_reference(m_responseBuffer, m_buffer->data(), m_buffer->size(),
                            &OutputPack::CleanUpData, buf.get()))
                    {
                        throw std::runtime_error{"[Mif::Net::Http::Detail::OutputPack] Failed to set data."};
                    }
                    buf.release();
                }

                void OutputPack::ReleaseNewRequest()
                {
                    m_newRequest.release();
                }

                Code OutputPack::GetCode() const
                {
                    return m_code;
                }

                std::string OutputPack::GetReason() const
                {
                    return m_reason;
                }

                Common::Buffer OutputPack::GetData() const
                {
                    if (m_buffer)
                        return *m_buffer.get();
                    return {};
                }

                void OutputPack::SetCode(Code code)
                {
                    m_code = code;
                }

                void OutputPack::SetReason(std::string const &reason)
                {
                    m_reason = reason;
                }

                void OutputPack::SetHeader(std::string const &key, std::string const &value)
                {
                    if (key.empty())
                        throw std::invalid_argument{"[Mif::Net::Http::Detail::OutputPack::AddHeader] Key must not be empty."};
                    if (value.empty())
                        throw std::invalid_argument{"[Mif::Net::Http::Detail::OutputPack::AddHeader] Value must not be empty."};

                    if (evhttp_add_header(m_headers, key.c_str(), value.c_str()))
                    {
                        throw std::runtime_error{"[Mif::Net::Http::Detail::OutputPack::AddHeader] Failed to set header. "
                            "Key: \"" + key + "\"\tValue: \"" + value + "\""};
                    }
                }

                void OutputPack::SetData(Common::Buffer buffer)
                {
                    m_buffer = std::make_shared<Common::Buffer>(std::move(buffer));
                }

                void OutputPack::SetData(Common::BufferPtr buffer)
                {
                    m_buffer = buffer;
                }

                void OutputPack::CleanUpData(void const *data, size_t datalen, void *extra)
                {
                    Common::Unused(data);
                    Common::Unused(datalen);
                    std::unique_ptr<Common::BufferPtr>{reinterpret_cast<Common::BufferPtr *>(extra)};
                }

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
