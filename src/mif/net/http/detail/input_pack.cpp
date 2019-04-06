//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// STD
#include <cstdlib>
#include <stdexcept>

// EVENT
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

// MIF
#include "../../../../../include/mif/common/log.h"

// THIS
#include "input_pack.h"
#include "utility.h"

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

                InputPack::InputPack(evhttp_request *request)
                    : m_request{request}
                {
                    if (!m_request)
                        throw std::invalid_argument{"[Mif::Net::Http::Detail::InputPack] Empty request pointer."};
                    auto const *uri = evhttp_request_get_uri(m_request);
                    if (!uri)
                        throw std::runtime_error{"[Mif::Net::Http::Detail::InputPack] Failed to get uri."};
                    m_uri.reset(evhttp_uri_parse_with_flags(uri, EVHTTP_URI_NONCONFORMANT));
                    if (!m_uri)
                        throw std::runtime_error{"[Mif::Net::Http::Detail::InputPack] Failed to parse uri."};
                }

                Method::Type InputPack::GetType() const
                {
                    auto const type = evhttp_request_get_command(m_request);

                    switch (type)
                    {
                    case EVHTTP_REQ_GET :
                        return Method::Type::Get;
                    case EVHTTP_REQ_POST :
                        return Method::Type::Post;
                    case EVHTTP_REQ_HEAD :
                        return Method::Type::Head;
                    case EVHTTP_REQ_PUT :
                        return Method::Type::Put;
                    case EVHTTP_REQ_DELETE :
                        return Method::Type::Delete;
                    case EVHTTP_REQ_OPTIONS :
                        return Method::Type::Options;
                    case EVHTTP_REQ_TRACE :
                        return Method::Type::Trqce;
                    case EVHTTP_REQ_CONNECT :
                        return Method::Type::Connect;
                    case EVHTTP_REQ_PATCH :
                        return Method::Type::Patch;
                    default :
                        break;
                    }

                    throw std::runtime_error{"[Mif::Net::Http::Detail::InputPack::GetType] Unknown ty0e \""
                        + std::to_string(type) + "\""};
                }

                Code InputPack::GetCode() const
                {
                    return Utility::ConvertCode(evhttp_request_get_response_code(m_request));
                }

                std::string InputPack::GetReason() const
                {
                    return Utility::GetReasonString(GetCode());
                }

                std::string InputPack::GetHost() const
                {
                    return ToString(evhttp_request_get_host(m_request));
                }

                std::uint16_t InputPack::GetPort() const
                {
                    if (!m_uri)
                        return -1;
                    return evhttp_uri_get_port(m_uri.get());
                }

                std::string InputPack::GetSchema() const
                {
                    if (!m_uri)
                        return {};
                    return ToString(evhttp_uri_get_scheme(m_uri.get()));
                }

                std::string InputPack::GetUserInfo() const
                {
                    if (!m_uri)
                        return {};
                    return ToString(evhttp_uri_get_userinfo(m_uri.get()));
                }

                std::string InputPack::GetPath() const
                {
                    if (!m_uri)
                        return {};
                    auto const *path = evhttp_uri_get_path(m_uri.get());
                    if (!path)
                        return {};
                    std::unique_ptr<char, decltype(&std::free)> decoded{evhttp_uridecode(path, 0, nullptr), &std::free};
                    return ToString(decoded.get());
                }

                std::string InputPack::GetQuery() const
                {
                    if (!m_uri)
                        return {};
                    auto const *query = evhttp_uri_get_query(m_uri.get());
                    if (!query)
                        return {};
                    std::unique_ptr<char, decltype(&std::free)> decoded{evhttp_uridecode(query, 0, nullptr), &std::free};
                    return ToString(decoded.get());
                }

                std::string InputPack::GetFragment() const
                {
                    if (!m_uri)
                        return {};
                    return ToString(evhttp_uri_get_fragment(m_uri.get()));
                }

                InputPack::Params InputPack::GetParams() const
                {
                    auto const &query = GetQuery();
                    if (query.empty())
                        return {};

                    evkeyvalq params;
                    if (evhttp_parse_query_str(query.c_str(), &params))
                        return {};

                    Params result;
                    for (evkeyval *i = params.tqh_first ; i ; i = i->next.tqe_next)
                        result.insert(std::make_pair(ToString(i->key), ToString(i->value)));

                    return result;
                }

                InputPack::Headers InputPack::GetHeaders() const
                {
                    auto const *headers = evhttp_request_get_input_headers(m_request);
                    if (!headers)
                        return {};

                    Headers result;
                    for (evkeyval *i = headers->tqh_first ; i ; i = i->next.tqe_next)
                        result.insert(std::make_pair(ToString(i->key), ToString(i->value)));

                    return result;
                }

                Common::Buffer InputPack::GetData() const
                {
                    auto *inputBuffer = evhttp_request_get_input_buffer(m_request);
                    if (!inputBuffer)
                        return {};
                    auto const length = evbuffer_get_length(inputBuffer);
                    if (!length)
                        return {};
                    Common::Buffer buffer(length);
                    if (evbuffer_copyout(inputBuffer, &buffer[0], length) != static_cast<ev_ssize_t>(length))
                        throw std::runtime_error{"[Mif::Net::Http::Detail::InputPack::GetData] Failed to copy buffer."};
                    return buffer;
                }

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
