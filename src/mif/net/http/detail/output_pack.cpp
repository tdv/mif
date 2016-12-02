//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

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
                    {
                        std::unique_ptr<Common::Buffer> buffer{!m_buffer.empty() ? new Common::Buffer{std::move(m_buffer)} : nullptr};
                        auto *data = buffer ? buffer->data() : nullptr;
                        auto const size = buffer ? buffer->size() : 0;
                        if (evbuffer_add_reference(m_responseBuffer, data, size, &OutputPack::CleanUpData, buffer.get()))
                        {
                            throw std::runtime_error{"[Mif::Net::Http::Detail::OutputPack] Failed to set data."};
                        }
                        buffer.release();
                    }

                    auto const code = Utility::ConvertCode(m_code);
                    evhttp_send_reply(m_request, code, m_reason.empty() ? Utility::GetReasonString(m_code) : m_reason.c_str(), m_responseBuffer);
                }

                evhttp_request* OutputPack::GetRequest()
                {
                    return m_request;
                }

                void OutputPack::ReleaseNewRequest()
                {
                    m_newRequest.release();
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
                    m_buffer = std::move(buffer);
                }

                void OutputPack::CleanUpData(void const *data, size_t datalen, void *extra)
                {
                    (void)data;
                    (void)datalen;
                    delete reinterpret_cast<Common::Buffer *>(extra);
                }

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
