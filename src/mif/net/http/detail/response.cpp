//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <memory>
#include <stdexcept>

// THIS
#include "response.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                Response::Response(evhttp_request *request)
                    : m_request{request}
                {
                    if (!m_request)
                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Response] Empty request pointer."};

                    if (!(m_responseBuffer = evhttp_request_get_output_buffer(m_request)))
                        throw std::runtime_error{"[Mif::Net::Http::Detail::Response] Failed to get output buffer."};

                    if (!(m_headers = evhttp_request_get_output_headers(m_request)))
                        throw std::runtime_error{"[Mif::Net::Http::Detail::Response] Failed to get output headers."};
                }

                void Response::Send()
                {
                    {
                        std::unique_ptr<Common::Buffer> buffer{!m_buffer.empty() ? new Common::Buffer{std::move(m_buffer)} : nullptr};
                        auto *data = buffer ? buffer->data() : nullptr;
                        auto const size = buffer ? buffer->size() : 0;
                        if (evbuffer_add_reference(m_responseBuffer, data, size, &Response::CleanUpData, buffer.get()))
                        {
                            throw std::runtime_error{"[Mif::Net::Http::Detail::Response] Failed to set data."};
                        }
                        buffer.release();
                    }

                    auto const code = ConvertCode(m_code);
                    evhttp_send_reply(m_request, code, m_reason.c_str(), m_responseBuffer);
                }

                void Response::SetCode(Code code)
                {
                    m_code = code;
                }

                void Response::SetReason(std::string const &reason)
                {
                    m_reason = reason;
                }

                void Response::SetHeader(std::string const &key, std::string const &value)
                {
                    if (key.empty())
                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Response::AddHeader] Key must not be empty."};
                    if (value.empty())
                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Response::AddHeader] Value must not be empty."};

                    if (evhttp_add_header(m_headers, key.c_str(), value.c_str()))
                    {
                        throw std::runtime_error{"[Mif::Net::Http::Detail::Response::AddHeader] Failed to set header. "
                            "Key: \"" + key + "\"\tValue: \"" + value + "\""};
                    }
                }

                void Response::SetData(Common::Buffer buffer)
                {
                    m_buffer = std::move(buffer);
                }

                void Response::CleanUpData(void const *data, size_t datalen, void *extra)
                {
                    (void)data;
                    (void)datalen;
                    delete reinterpret_cast<Common::Buffer *>(extra);
                }

                int Response::ConvertCode(Code code) const
                {
                    switch (code)
                    {
                    case Code::Ok :
                        return HTTP_OK;
                    case Code::NoContent :
                        return HTTP_NOCONTENT;
                    case Code::MovePerm :
                        return HTTP_MOVEPERM;
                    case Code::MoveTemp :
                        return HTTP_MOVETEMP;
                    case Code::NotModified :
                        return HTTP_NOTMODIFIED;
                    case Code::BadRequest :
                        return HTTP_BADREQUEST;
                    case Code::NotFound :
                        return HTTP_NOTFOUND;
                    case Code::BadMethod :
                        return HTTP_BADMETHOD;
                    case Code::Internal :
                        return HTTP_INTERNAL;
                    case Code::NotImplemented :
                        return HTTP_NOTIMPLEMENTED;
                    case Code::Unavaliable :
                        return HTTP_SERVUNAVAIL;
                    default :
                        break;
                    }

                    throw std::invalid_argument{"[Mif::Net::Http::Detail::Response::ConvertCode] Unknowd HTTP code."};
                }

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
