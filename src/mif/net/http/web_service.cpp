//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// C
#include <string.h>

// MIF
#include "mif/common/log.h"
#include "mif/common/unused.h"
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
                    PreProcessRequest(request);
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
                                auto const keepAlive = headers.find(Constants::Header::Request::Connection::Value);
                                if (keepAlive != std::end(headers))
                                {
                                    auto const &value = keepAlive->second;
                                    auto const *keepAliveValue = Constants::Value::Connection::KeepAlive::Value;
                                    auto const len = strlen(keepAliveValue);
                                    if (value.length() == len && !strncasecmp(value.c_str(),  keepAliveValue, len))
                                    {
                                        response.SetHeader(Constants::Header::Response::Connection::Value,
                                                Constants::Value::Connection::KeepAlive::Value);
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
                        OnExceptionResponse(request, response, Code::BadMethod,
                                std::make_exception_ptr(std::runtime_error{"Handler not found."}));
                    }

                    PostProcessResponse(response);
                }
                catch (std::invalid_argument const &e)
                {
                    ++m_statistics.general.bad;
                    OnExceptionResponse(request, response, Code::BadRequest, std::current_exception());
                }
                catch (std::exception const &e)
                {
                    ++m_statistics.general.bad;

                    MIF_LOG(Warning) << "[Mif::Net::Http::WebService::OnRequest] "
                        << "Failed to process request. Error: " << e.what();

                    OnExceptionResponse(request, response, Code::Internal, std::current_exception());
                }
                catch (...)
                {
                    ++m_statistics.general.bad;

                    MIF_LOG(Error) << "[Mif::Net::Http::WebService::OnRequest] "
                        << "Failed to process request. Error: unknown";

                    OnExceptionResponse(request, response, Code::Internal,
                            std::make_exception_ptr(std::runtime_error{"Unknown exception."}));
                }
            }

            void WebService::OnExceptionResponse(IInputPack const &request, IOutputPack &response,
                                                 Code code, std::exception_ptr exception)
            {
                response.SetHeader(Constants::Header::Response::Connection::Value,
                        Constants::Value::Connection::Close::Value);

                std::string message;
                try
                {
                    if (exception)
                        std::rethrow_exception(exception);
                }
                catch (std::exception const &e)
                {
                    message = e.what();
                }

                OnException(request, response, exception, code, message);

                response.SetCode(code);
                response.SetData({std::begin(message), std::end(message)});
            }

            void WebService::OnException(IInputPack const &request, IOutputPack &response, std::exception_ptr exception,
                                         Code &code, std::string &message) const
            {
                Common::Unused(request);
                Common::Unused(response);
                OnException(exception, code, message);
            }

            void WebService::OnException(std::exception_ptr exception, Code &code, std::string &message) const
            {
                Common::Unused(exception);
                Common::Unused(code);
                Common::Unused(message);
            }

            void WebService::PreProcessRequest(IInputPack const &request)
            {
                Common::Unused(request);
            }

            void WebService::PostProcessResponse(IOutputPack &response)
            {
                Common::Unused(response);
            }

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
