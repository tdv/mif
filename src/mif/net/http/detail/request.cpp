//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// EVENT
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

// THIS
#include "request.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {
                namespace
                {

                    inline std::string ToString(char const *str)
                    {
                        return {str ? str : ""};
                    }

                }   // namespace

                Request::Request(evhttp_request *request)
                    : m_request{request}
                {
                    if (!m_request)
                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Request] Empty request pointer."};

                    auto const *uri = evhttp_request_get_uri(m_request);
                    if (!uri)
                        throw std::runtime_error{"[Mif::Net::Http::Detail::Request] Failed to get uri."};
                    m_uri.reset(evhttp_uri_parse(uri));
                    if (!m_uri)
                        throw std::runtime_error{"[Mif::Net::Http::Detail::Request] Failed to parse uri."};
                }

                Request::Type Request::GetType() const
                {
                    switch (evhttp_request_get_command(m_request))
                    {
                    case EVHTTP_REQ_GET :
                        return Type::Get;
                    case EVHTTP_REQ_POST :
                        return Type::Post;
                    case EVHTTP_REQ_HEAD :
                        return Type::Head;
                    case EVHTTP_REQ_PUT :
                        return Type::Put;
                    case EVHTTP_REQ_DELETE :
                        return Type::Delete;
                    case EVHTTP_REQ_OPTIONS :
                        return Type::Options;
                    case EVHTTP_REQ_TRACE :
                        return Type::Trqce;
                    case EVHTTP_REQ_CONNECT :
                        return Type::Connect;
                    case EVHTTP_REQ_PATCH :
                        return Type::Patch;
                    default :
                        break;
                    }
                    return {};
                }

                std::string Request::GetHost() const
                {
                    return ToString(evhttp_request_get_host(m_request));
                }

                std::string Request::GetSchema() const
                {
                    if (!m_uri)
                        return {};
                    return ToString(evhttp_uri_get_scheme(m_uri.get()));
                }

                std::string Request::GetPath() const
                {
                    if (!m_uri)
                        return {};
                    return ToString(evhttp_uri_get_path(m_uri.get()));
                }

                std::string Request::GetQuery() const
                {
                    if (!m_uri)
                        return {};
                    return ToString(evhttp_uri_get_query(m_uri.get()));
                }

                std::string Request::GetFragment() const
                {
                    if (!m_uri)
                        return {};
                    return ToString(evhttp_uri_get_fragment(m_uri.get()));
                }

                Request::Params Request::GetParams() const
                {
                    auto const &query = GetQuery();
                    if (query.empty())
                        return {};

                    evkeyvalq params;
                    if (evhttp_parse_query_str(query.c_str(), &params))
                        throw std::runtime_error{"[Mif::Net::Http::Detail::Request] Failed to parse query."};

                    Params result;
                    for (evkeyval *i = params.tqh_first ; i ; i = i->next.tqe_next)
                        result.insert(std::make_pair(ToString(i->key), ToString(i->value)));

                    return result;
                }

                Request::Headers Request::GetHeaders() const
                {
                    auto const *headers = evhttp_request_get_input_headers(m_request);
                    if (!headers)
                        return {};

                    Headers result;
                    for (evkeyval *i = headers->tqh_first ; i ; i = i->next.tqe_next)
                        result.insert(std::make_pair(ToString(i->key), ToString(i->value)));

                    return result;
                }

                Common::Buffer Request::GetData() const
                {
                    auto *inputBuffer = evhttp_request_get_input_buffer(m_request);
                    if (!inputBuffer)
                        return {};
                    auto const length = evbuffer_get_length(inputBuffer);
                    if (!length)
                        return {};
                    Common::Buffer buffer(length);
                    if (evbuffer_copyout(inputBuffer, &buffer[0], length) != static_cast<ev_ssize_t>(length))
                        throw std::runtime_error{"[Mif::Net::Http::Detail::Request::GetData] Failed to copy buffer."};
                    return buffer;
                }

                void Request::Send()
                {
                    throw std::runtime_error{"[Mif::Net::Http::Detail::Request::Send] Not implemented."};
                }

                /*int Request::ConvertCode(Code code) const
                {
                    enum class Code
                    {
                        Ok,
                        NoContent,
                        MovePerm,
                        MoveTemp,
                        NotModified,
                        BadRequest,
                        NotFound,
                        BadMethod,
                        Internal,
                        NotImplemented,
                        Unavaliable
                    };
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

                    throw std::invalid_argument{"[Mif::Net::Http::Detail::Request::ConvertCode] Unknowd HTTP code."};
                }*/

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
