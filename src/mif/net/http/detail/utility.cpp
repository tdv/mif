//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// EVENT
#include <event2/http.h>

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
                namespace Utility
                {

                    int ConvertMethodType(Method::Type type)
                    {
                        switch (type)
                        {
                        case Method::Type::Get :
                            return EVHTTP_REQ_GET;
                        case Method::Type::Post :
                            return EVHTTP_REQ_POST;
                        case Method::Type::Head :
                            return EVHTTP_REQ_HEAD;
                        case Method::Type::Put :
                            return EVHTTP_REQ_PUT;
                        case Method::Type::Delete :
                            return EVHTTP_REQ_DELETE;
                        case Method::Type::Options :
                            return EVHTTP_REQ_OPTIONS;
                        case Method::Type::Trqce :
                            return EVHTTP_REQ_TRACE;
                        case Method::Type::Connect :
                            return EVHTTP_REQ_CONNECT;
                        case Method::Type::Patch :
                            return EVHTTP_REQ_PATCH;
                        default :
                            break;
                        }

                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Utility::ConvertMethodType] "
                            "Unknown method type."};
                    }

                    int ConvertAllowedMethods(Methods const &allowedMethods)
                    {
                        int methods = 0;

                        for (auto const &i : allowedMethods)
                            methods |= ConvertMethodType(i);

                        return methods;
                    }

                    char const* GetReasonString(IOutputPack::Code code)
                    {
                        switch (code)
                        {
                        case IOutputPack::Code::Ok :
                            return "OK";
                        case IOutputPack::Code::NoContent :
                            return "Request does not have content";
                        case IOutputPack::Code::MovePerm :
                            return "The uri moved permanently";
                        case IOutputPack::Code::MoveTemp :
                            return "The uri moved temporarily";
                        case IOutputPack::Code::NotModified :
                            return "Page was not modified from last";
                        case IOutputPack::Code::BadRequest :
                            return "Invalid http request was made";
                        case IOutputPack::Code::NotFound :
                            return "Could not find content for uri";
                        case IOutputPack::Code::BadMethod :
                            return "Method not allowed for this uri";
                        case IOutputPack::Code::Internal :
                            return "Internal error";
                        case IOutputPack::Code::NotImplemented :
                            return "Not implemented";
                        case IOutputPack::Code::Unavaliable :
                            return "The server is not available";
                        default :
                            break;
                        }

                        return "";
                    }

                    int ConvertCode(IOutputPack::Code code)
                    {
                        switch (code)
                        {
                        case IOutputPack::Code::Ok :
                            return HTTP_OK;
                        case IOutputPack::Code::NoContent :
                            return HTTP_NOCONTENT;
                        case IOutputPack::Code::MovePerm :
                            return HTTP_MOVEPERM;
                        case IOutputPack::Code::MoveTemp :
                            return HTTP_MOVETEMP;
                        case IOutputPack::Code::NotModified :
                            return HTTP_NOTMODIFIED;
                        case IOutputPack::Code::BadRequest :
                            return HTTP_BADREQUEST;
                        case IOutputPack::Code::NotFound :
                            return HTTP_NOTFOUND;
                        case IOutputPack::Code::BadMethod :
                            return HTTP_BADMETHOD;
                        case IOutputPack::Code::Internal :
                            return HTTP_INTERNAL;
                        case IOutputPack::Code::NotImplemented :
                            return HTTP_NOTIMPLEMENTED;
                        case IOutputPack::Code::Unavaliable :
                            return HTTP_SERVUNAVAIL;
                        default :
                            break;
                        }

                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Utility::ConvertCode] Unknowd HTTP code."};
                    }

                }   // namespace Utility

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
