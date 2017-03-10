//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_WEB_SERVICE_H__
#define __MIF_NET_HTTP_WEB_SERVICE_H__

// STD
#include <atomic>
#include <cstdint>
#include <functional>
#include <clocale>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

// BOOST
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// MIF
#include "mif/common/crc32.h"
#include "mif/net/http/iweb_service.h"
#include "mif/net/http/request_handler.h"
#include "mif/serialization/traits.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            struct UrlParamConverter final
            {
                template <typename T>
                static typename std::enable_if<std::is_same<T, std::string>::value, T>::type
                Convert(std::string const &value)
                {
                    return value;
                }

                template <typename T>
                static typename std::enable_if<std::is_integral<T>::value, T>::type
                Convert(std::string const &value)
                {
                    return static_cast<T>(std::stoll(value));
                }

                template <typename T>
                static typename std::enable_if<std::is_floating_point<T>::value, T>::type
                Convert(std::string const &value)
                {
                    return static_cast<T>(std::stod(value));
                }

                template <typename T>
                static typename std::enable_if<Serialization::Traits::IsIterable<T>(), T>::type
                Convert(std::string const &value)
                {
                    std::set<std::string> items;
                    boost::split(items, value, boost::is_any_of(";"));
                    T seq;
                    std::transform(std::begin(items), std::end(items), std::inserter(seq, std::begin(seq)),
                            std::ptr_fun(UrlParamConverter::template Convert<typename T::value_type>));
                    return seq;
                }

                template <typename T>
                static typename std::enable_if<std::is_same<T, boost::posix_time::ptime::date_type>::value, T>::type
                Convert(std::string const &value)
                {
                    std::istringstream stream{value};
                    stream.imbue(std::locale(std::locale::classic(),
                            new boost::posix_time::time_input_facet("%Y.%m.%d")));
                    boost::posix_time::ptime pt;
                    stream >> pt;
                    return pt.date();
                }

                template <typename T>
                static typename std::enable_if<std::is_same<T, boost::posix_time::ptime::time_duration_type>::value, T>::type
                Convert(std::string const &value)
                {
                    std::istringstream stream{value};
                    stream.imbue(std::locale(std::locale::classic(),
                            new boost::posix_time::time_input_facet("%H:%M:%S")));
                    boost::posix_time::ptime pt;
                    stream >> pt;
                    return pt.time_of_day();
                }

                template <typename T>
                static typename std::enable_if<std::is_same<T, boost::posix_time::ptime>::value, T>::type
                Convert(std::string const &value)
                {
                    std::istringstream stream{value};
                    stream.imbue(std::locale(std::locale::classic(),
                            new boost::posix_time::time_input_facet("%Y.%m.%d %H:%M:%S")));
                    boost::posix_time::ptime pt;
                    stream >> pt;
                    return pt;
                }
            };

            class WebService
                : public Service::Inherit<IWebService>
            {
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
                    ExtractType<T> GetPrm(IInputPack::Params const &params)
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

                    // IWebServiceHandler
                    virtual void OnRequest(IInputPack const &request, IOutputPack &response) override final
                    {
                        (m_object->*m_method)(GetPrm<Args>(request.GetParams()) ... );
                        (void)response;
                    }
                };

            protected:
                WebService() = default;
                ~WebService() = default;

                template <typename C, typename R, typename ... Args>
                typename std::enable_if<std::is_base_of<WebService, C>::value, void>::type
                AddHandler(std::string const &resource, C *object, R (C::*method)(Args ...))
                {
                    IWebServiceHandlerPtr hanlder{new WebServiceHandler<C, R, Args ... >{object, method}};
                    m_handlers.emplace(resource, std::move(hanlder));
                    m_statistics.resources[resource];
                }

                template
                <
                    typename T,
                    std::uint32_t ID,
                    typename TConverter = UrlParamConverter
                >
                class Prm final
                {
                public:
                    using Type = T;

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

                    static constexpr std::uint32_t Id = ID;

                    std::string m_name;
                    std::unique_ptr<T> const m_value;

                    Prm() = default;
                    Prm(Prm &&) = default;
                    Prm(Prm const &) = delete;

                    Prm(std::string const &name, std::string const &value)
                        : m_name{name}
                        , m_value{new T{TConverter::template Convert<T>(value)}}
                    {
                    }
                };

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

            private:
                using IWebServiceHandlerPtr = std::unique_ptr<IWebServiceHandler>;
                using Handlers = std::map<std::string, IWebServiceHandlerPtr>;

                Statistics m_statistics;
                Handlers m_handlers;

                //--------------------------------------------------------------------------------------------------
                // IWebService
                virtual void OnRequest(IInputPack const &request, IOutputPack &response) override final;

                //--------------------------------------------------------------------------------------------------
                void OnExceptionResponse(IOutputPack &pack, Code code, std::string const &message);
            };

            template <typename TService, typename ... TArgs>
            inline typename std::enable_if<std::is_base_of<WebService, TService>::value, ServerHandler>::type
            MakeWebService(TArgs && ... args)
            {
                auto service = Service::Make<TService, IWebService>(std::forward<TArgs>(args) ... );
                auto handler = std::bind(&IWebService::OnRequest, service, std::placeholders::_1, std::placeholders::_2);
                return handler;
            }

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif


#endif  // !__MIF_NET_HTTP_WEB_SERVICE_H__
