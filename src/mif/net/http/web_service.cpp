//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// C
#include <string.h>

// MIF
#include "mif/common/log.h"
#include "mif/net/http/web_service.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            WebService::Statistics const& WebService::GetStatistics() const
            {
                return m_statistics;
            }

            void WebService::OnRequest(IInputPack const &request, IOutputPack &response)
            {
                try
                {
                    ++m_statistics.general.total;
                    auto const path = request.GetPath();
                    auto const iter = m_handlers.find(path);
                    if (iter != std::end(m_handlers))
                    {
                        try
                        {
                            ++m_statistics.resources[path].total;
                            iter->second->OnRequest(request, response);
                            {
                                auto const headers = request.GetHeaders();
                                auto const keepAlive = headers.find(Constants::Header::Connection::GetString());
                                if (keepAlive != std::end(headers))
                                {
                                    auto const &value = keepAlive->second;
                                    auto const *keepAliveValue = Constants::Value::Connection::KeepAlive::GetString();
                                    auto const len = strlen(keepAliveValue);
                                    if (value.length() == len && !strncasecmp(value.c_str(),  keepAliveValue, len))
                                    {
                                        response.SetHeader(Constants::Header::Connection::GetString(),
                                                Constants::Value::Connection::KeepAlive::GetString());
                                    }
                                }
                            }
                        }
                        catch (...)
                        {
                            ++m_statistics.resources[path].bad;
                            throw;
                        }
                    }
                    else
                    {
                        ++m_statistics.general.bad;
                        OnExceptionResponse(response, Code::BadMethod, "Handler not found.");
                    }
                }
                catch (std::invalid_argument const &e)
                {
                    ++m_statistics.general.bad;
                    OnExceptionResponse(response, Code::BadRequest, e.what());
                }
                catch (std::exception const &e)
                {
                    ++m_statistics.general.bad;

                    MIF_LOG(Warning) << "[Mif::Net::Http::WebService::OnRequest] "
                        << "Failed to process request. Error: " << e.what();

                    OnExceptionResponse(response, Code::Internal, e.what());
                }
                catch (...)
                {
                    ++m_statistics.general.bad;

                    MIF_LOG(Error) << "[Mif::Net::Http::WebService::OnRequest] "
                        << "Failed to process request. Error: unknown";

                    OnExceptionResponse(response, Code::Internal, "Unknown exception.");
                }
            }

            void WebService::OnExceptionResponse(IOutputPack &pack, Code code, std::string const &message)
            {
                pack.SetHeader(Constants::Header::Connection::GetString(),
                    Constants::Value::Connection::Close::GetString());
                pack.SetCode(code);
                pack.SetData({std::begin(message), std::end(message)});
            }

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
