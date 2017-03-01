//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include "mif/common/log.h"
#include "mif/net/http/web_service.h"

// THIS
#include "detail/constants.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            void WebService::OnRequest(IInputPack const &request, IOutputPack &response)
            {
                try
                {
                    auto const path = request.GetPath();
                    auto const iter = m_handlers.find(path);
                    if (iter != std::end(m_handlers))
                        iter->second->OnRequest(request, response);
                    else
                        OnExceptionResponse(response, Code::BadMethod, "Handler not found.");
                }
                catch (std::invalid_argument const &e)
                {
                    OnExceptionResponse(response, Code::BadRequest, e.what());
                }
                catch (std::exception const &e)
                {
                    MIF_LOG(Warning) << "[Mif::Net::Http::WebService::OnRequest] "
                        << "Failed to process request. Error: " << e.what();

                    OnExceptionResponse(response, Code::Internal, e.what());
                }
                catch (...)
                {
                    MIF_LOG(Error) << "[Mif::Net::Http::WebService::OnRequest] "
                        << "Failed to process request. Error: unknown";

                    OnExceptionResponse(response, Code::Internal, "Unknown exception.");
                }
            }

            void WebService::OnExceptionResponse(IOutputPack &pack, Code code, std::string const &message)
            {
                pack.SetHeader(Detail::Constants::Header::Connection::GetString(),
                    Detail::Constants::Value::Connection::Close::GetString());
                pack.SetCode(code);
                pack.SetData({std::begin(message), std::end(message)});
            }

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
