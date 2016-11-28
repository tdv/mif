//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
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
                }

                void Response::Send()
                {
                    throw std::runtime_error{"[Mif::Net::Http::Detail::Response::Send] Not implemented."};
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
