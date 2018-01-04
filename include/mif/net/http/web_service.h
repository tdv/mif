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
#include "mif/net/http/constants.h"
#include "mif/net/http/converter/content/plain_text.h"
#include "mif/net/http/converter/url/param.h"
#include "mif/net/http/iweb_service.h"
#include "mif/net/http/request_handler.h"
#include "mif/net/http/serializer/plain_text.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            class WebService
                : public Service::Inherit<IWebService>
            {
            private:
                template <typename, typename, typename ... >
                class WebServiceHandler;

            protected:
                WebService() = default;
                ~WebService() = default;

                template <typename>
                class Result;

                template
                <
                    typename T,
                    std::uint32_t ID,
                    typename TConverter = Converter::Url::Param
                >
                class Prm final
                {
                public:
                    using PrmType = Prm<T, ID, TConverter>;
                    using Type = T;
                    static constexpr std::uint32_t Id = ID;
                    using Converter = TConverter;

                    explicit operator bool() const noexcept
                    {
                        return !!m_value;
                    }

                    explicit operator T const & () const
                    {
                        if (!*this)
                            throw std::runtime_error{"[Mif::Net::Http::WebService::Prm] \"" + m_name + "\" has no value."};
                        return *m_value;
                    }

                    Type const& Get() const
                    {
                        return operator T const  & ();
                    }

                private:
                    template <typename, typename, typename ... >
                    friend class WebServiceHandler;

                    std::string m_name;
                    std::unique_ptr<T> const m_value;

                    Prm() = default;
                    Prm(Prm &&) = default;
                    Prm(Prm const &) = delete;

                    Prm(std::string const &name, std::string const &value)
                        : m_name{name}
                        , m_value{new T{Converter::template Convert<T>(value)}}
                    {
                    }
                };

                class Params
                {
                public:
                    using Type = std::map<std::string, std::string>;

                    Type const& Get() const
                    {
                        return m_params;
                    }

                private:
                    template <typename, typename, typename ... >
                    friend class WebServiceHandler;

                    Type const &m_params;

                    Params(Type const &params)
                        : m_params{params}
                    {
                    }
                };

                class Headers
                {
                public:
                    using Type = std::map<std::string, std::string>;

                    Type const& Get() const
                    {
                        return m_headers;
                    }

                private:
                    template <typename, typename, typename ... >
                    friend class WebServiceHandler;

                    template <typename>
                    friend class Result;

                    Type const &m_headers;

                    Headers(Type const &headers)
                        : m_headers{headers}
                    {
                    }
                };

                template <typename T, typename TConverter = Converter::Content::PlainText>
                class Content final
                {
                public:
                    using ContentType = Content<T, TConverter>;
                    using Type = T;
                    using Converter = TConverter;

                    explicit operator bool() const noexcept
                    {
                        return !!m_value;
                    }

                    explicit operator T const & () const
                    {
                        if (!*this)
                            throw std::runtime_error{"[Mif::Net::Http::WebService::Content] No content."};
                        return *m_value;
                    }

                    Type const& Get() const
                    {
                        return operator T const  & ();
                    }

                private:
                    template <typename, typename, typename ... >
                    friend class WebServiceHandler;

                    std::unique_ptr<T> const m_value;

                    Content() = default;
                    Content(Content &&) = default;
                    Content(Content const &) = delete;

                    Content(Common::Buffer const &value)
                        : m_value{new Type{Converter::template Convert<Type>(value)}}
                    {
                    }
                };

                template <typename TSerializer = Serializer::PlainText>
                class Result final
                {
                public:
                    template <typename T>
                    Result(T const &data, std::string const &contentType = TSerializer::GetContentType())
                        : m_value{TSerializer::Serialize(data)}
                        , m_headers{{Constants::Header::Response::ContentType::Value, contentType}}
                    {
                    }

                    template <typename T>
                    Result(T const &data, Headers::Type const &headers,
                            std::string const &contentType = TSerializer::GetContentType())
                        : m_value{TSerializer::Serialize(data)}
                        , m_headers{headers}
                    {
                        if (m_headers.find(Constants::Header::Response::ContentType::Value) == std::end(m_headers))
                            m_headers.emplace(Constants::Header::Response::ContentType::Value, contentType);
                    }

                    template <typename TOther>
                    Result(Result<TOther> const &other)
                        : m_value{other.m_value}
                        , m_headers{other.m_headers}
                    {
                    }

                    Common::Buffer GetValue()
                    {
                        return std::move(m_value);
                    }

                    Headers const GetHeaders() const
                    {
                        return m_headers;
                    }

                private:
                    template <typename>
                    friend class Result;

                    Common::Buffer m_value;
                    Headers::Type m_headers;
                };

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
                    using ExtractType = typename std::remove_reference
                            <
                                typename std::remove_const<T>::type
                            >::type;

                    template <typename T>
                    typename ExtractType<T>::PrmType GetPrm(IInputPack::Params const &params,
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
                    typename ExtractType<T>::Params GetPrm(IInputPack::Params const &params,
                            IInputPack::Headers const &, Common::Buffer const &) const
                    {
                        return {params};
                    }

                    template <typename T>
                    typename ExtractType<T>::Headers GetPrm(IInputPack::Params const &,
                            IInputPack::Headers const &headers, Common::Buffer const &) const
                    {
                        return {headers};
                    }

                    template <typename T>
                    typename ExtractType<T>::ContentType GetPrm(IInputPack::Params const &,
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
                        (m_object->*m_method)(GetPrm<Args>(params, headers, content) ... );
                    }

                    template <typename T>
                    typename std::enable_if<!std::is_same<T, void>::value, void>::type
                    ProcessRequest(IInputPack const &request, IOutputPack &response)
                    {
                        auto const params = request.GetParams();
                        auto const headers = request.GetHeaders();
                        auto const content = request.GetData();
                        Result<> res{(m_object->*m_method)(GetPrm<Args>(params, headers, content) ... )};
                        for (auto const &header : res.GetHeaders().Get())
                            response.SetHeader(header.first, header.second);
                        response.SetData(std::move(res.GetValue()));
                    }
                };

            protected:

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
