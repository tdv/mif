//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_WEB_SERVICE_H__
#define __MIF_NET_HTTP_WEB_SERVICE_H__

// STD
#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

// MIF
#include "mif/common/crc32.h"
#include "mif/net/http/converter/content/plain_text.h"
#include "mif/net/http/converter/url/param.h"
#include "mif/net/http/detail/content.h"
#include "mif/net/http/detail/params.h"
#include "mif/net/http/detail/prm.h"
#include "mif/net/http/detail/param_pack.h"
#include "mif/net/http/detail/result.h"
#include "mif/net/http/iweb_service.h"
#include "mif/net/http/request_handler.h"
#include "mif/net/http/serializer/plain_text.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            class WebService
                : public Service::Inherit<IWebService>
            {
            protected:
                WebService() = default;
                ~WebService() = default;

                template <typename T, std::uint32_t ID, typename TConverter = Converter::Url::Param>
                using Prm = Detail::Prm<T, ID, TConverter>;

                using Params = Detail::Params<Detail::Tag::Params>;
                using Headers = Detail::Params<Detail::Tag::Headers>;

                template <typename T, typename TConverter = Converter::Url::Param>
                using ParamPack = Detail::ParamPack<T, TConverter>;

                template <typename T, typename TConverter = Converter::Content::PlainText>
                using Content = Detail::Content<T, TConverter>;

                template <typename TSerializer = Serializer::PlainText>
                using Result = Detail::Result<TSerializer>;

                template <typename C, typename R, typename ... Args>
                typename std::enable_if<std::is_base_of<WebService, C>::value, void>::type
                AddHandler(std::string const &resource, C *object, R (C::*method)(Args ...))
                {
                    IWebServiceHandlerPtr hanlder{new WebServiceHandler<C, R, Args ... >{object, method}};
                    m_handlers.emplace(resource, std::move(hanlder));
                    m_statistics.resources[resource];
                }

                template <std::size_t N>
                static constexpr std::uint32_t Name(char const (&name)[N])
                {
                    return Common::Crc32(name);
                }

                struct Statistics
                {
                    struct ItemCounter
                    {
                        std::atomic_ullong total{0};
                        std::atomic_ullong bad{0};
                    };

                    using Resources = std::map<std::string, ItemCounter>;

                    ItemCounter general;
                    Resources resources;
                };

                Statistics const& GetStatistics() const;

                virtual void OnException(IInputPack const &request, IOutputPack &response, std::exception_ptr exception,
                        Code &code, std::string &message) const;
                virtual void OnException(std::exception_ptr exception, Code &code, std::string &message) const;

                virtual void PreProcessRequest(IInputPack const &request);
                virtual void PostProcessResponse(IOutputPack &response);

            private:
                struct IWebServiceHandler
                {
                    virtual ~IWebServiceHandler() = default;
                    virtual void OnRequest(IInputPack const &request, IOutputPack &response) = 0;
                };

                template <typename C, typename R, typename ... Args>
                class WebServiceHandler final
                    : public IWebServiceHandler
                {
                public:
                    WebServiceHandler(C *object, R (C::*method)(Args ... ))
                        : m_object{object}
                        , m_method{method}
                    {
                    }

                private:
                    using Method = R (C::*)(Args ... );

                    C *m_object;
                    Method m_method;

                    template <typename T>
                    using ExtractType = typename std::decay<T>::type;

                    template <typename T>
                    typename ExtractType<T>::PrmType GetPrm(IInputPack const &, IInputPack::Params const &params,
                            IInputPack::Headers const &, Common::Buffer const &) const
                    {
                        for (auto const &i : params)
                        {
                            if (Common::Crc32str(i.first) == ExtractType<T>::Id)
                            {
                                return {i.first, i.second};
                            }
                        }
                        return {};
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<ExtractType<T>, Params>::value, ExtractType<T>>::type
                    GetPrm(IInputPack const &, IInputPack::Params const &params,
                            IInputPack::Headers const &, Common::Buffer const &) const
                    {
                        return {params};
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<ExtractType<T>, Headers>::value, ExtractType<T>>::type
                    GetPrm(IInputPack const &, IInputPack::Params const &,
                            IInputPack::Headers const &headers, Common::Buffer const &) const
                    {
                        return {headers};
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<ExtractType<T>, IInputPack>::value, ExtractType<T>>::type const &
                    GetPrm(IInputPack const &request, IInputPack::Params const &,
                            IInputPack::Headers const &, Common::Buffer const &) const
                    {
                        return request;
                    }

                    template <typename T>
                    typename std::enable_if<Detail::IsParamPack<T>(), ExtractType<T>>::type
                    GetPrm(IInputPack const &, IInputPack::Params const &params,
                            IInputPack::Headers const &, Common::Buffer const &) const
                    {
                        return {params};
                    }

                    template <typename T>
                    typename ExtractType<T>::ContentType GetPrm(IInputPack const &, IInputPack::Params const &,
                            IInputPack::Headers const &, Common::Buffer const &content) const
                    {
                        return {content};
                    }

                    // IWebServiceHandler
                    virtual void OnRequest(IInputPack const &request, IOutputPack &response) override final
                    {
                        ProcessRequest<R>(request, response);
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<T, void>::value, void>::type
                    ProcessRequest(IInputPack const &request, IOutputPack &)
                    {
                        auto const params = request.GetParams();
                        auto const headers = request.GetHeaders();
                        auto const content = request.GetData();
                        (m_object->*m_method)(GetPrm<Args>(request, params, headers, content) ... );
                    }

                    template <typename T>
                    typename std::enable_if<!std::is_same<T, void>::value, void>::type
                    ProcessRequest(IInputPack const &request, IOutputPack &response)
                    {
                        auto const params = request.GetParams();
                        auto const headers = request.GetHeaders();
                        auto const content = request.GetData();
                        Result<> res{(m_object->*m_method)(GetPrm<Args>(request, params, headers, content) ... )};
                        for (auto const &header : res.GetHeaders().Get())
                            response.SetHeader(header.first, header.second);
                        response.SetData(std::move(res.GetValue()));
                    }
                };

                using IWebServiceHandlerPtr = std::unique_ptr<IWebServiceHandler>;
                using Handlers = std::map<std::string, IWebServiceHandlerPtr>;

                Statistics m_statistics;
                Handlers m_handlers;

                //--------------------------------------------------------------------------------------------------
                // IWebService
                virtual void OnRequest(IInputPack const &request, IOutputPack &response) override final;

                //--------------------------------------------------------------------------------------------------
                void OnExceptionResponse(IInputPack const &request, IOutputPack &response,
                        Code code, std::exception_ptr exception);
            };

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_WEB_SERVICE_H__
