//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_SERIALIZATION_JSON_H__
#define __MIF_REMOTE_SERIALIZATION_JSON_H__

// STD
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

// BOOST
#include <boost/json.hpp>

// MIF
#include "mif/common/types.h"
#include "mif/remote/serialization/detail/tag.h"
#include "mif/serialization/json.h"

namespace Mif
{
    namespace Remote
    {
        namespace Serialization
        {
            namespace Json
            {

                class Serializer final
                {
                public:
                    template <typename ... TParams>
                    Serializer(bool isReques, std::string const &uuid,
                        std::string const &instanceId, std::string const &interfaceId,
                        std::string const &methodId, TParams && ... params)
                    {
                        m_value[Detail::Tag::Uuid::Value] = uuid;
                        m_value[Detail::Tag::Type::Value] = isReques ?
                            Detail::Tag::Request::Value :
                            Detail::Tag::Response::Value;
                        m_value[Detail::Tag::Instsnce::Value] = instanceId;
                        m_value[Detail::Tag::Interface::Value] = interfaceId;
                        m_value[Detail::Tag::Method::Value] = methodId;

                        PutParamsIfExists(std::forward<TParams>(params) ... );
                    }

                    template <typename ... TParams>
                    void PutParams(TParams && ... params)
                    {
                        if (auto iter = m_value.find(Detail::Tag::Param::Value); iter != std::end(m_value))
                            m_value.erase(iter);

                        auto const tuple = std::make_tuple(std::forward<TParams>(params) ... );
                        m_value[Detail::Tag::Param::Value] = ::Mif::Serialization::Json::Detail::ValueToJson(tuple);
                    }

                    void PutException(std::exception_ptr ex)
                    {
                        if (auto iter = m_value.find(Detail::Tag::Exception::Value); iter != std::end(m_value))
                            m_value.erase(iter);

                        try
                        {
                            std::rethrow_exception(ex);
                        }
                        catch (std::exception const &e)
                        {
                            m_value[Detail::Tag::Exception::Value] = e.what();
                        }
                        catch (...)
                        {
                            m_value[Detail::Tag::Exception::Value] = "Unknown exception.";
                        }
                    }

                    Common::Buffer GetBuffer()
                    {
                        auto data = boost::json::serialize(boost::json::value_from(m_value));
                        return {std::begin(data), std::end(data)};
                    }

                private:
                    boost::json::object m_value;

                    template <typename ... TParams>
                    typename std::enable_if<sizeof ... (TParams), void>::type
                    PutParamsIfExists(TParams && ... params)
                    {
                        PutParams(std::forward<TParams> (params) ... );
                    }

                    template <typename ... TParams>
                    typename std::enable_if< !sizeof ... (TParams), void>::type
                    PutParamsIfExists(TParams && ...)
                    {
                    }
                };

                class Deserializer final
                {
                public:
                    Deserializer(Common::Buffer buffer)
                    {
                        if (buffer.empty())
                            throw std::invalid_argument{"[Mif::Remote::Serialization::Json::Deserializer] Empty buffer."};

                        auto val = boost::json::parse(boost::string_view{buffer.data(), buffer.size()});

                        if (val.is_null())
                            throw std::invalid_argument{"[Mif::Remote::Serialization::Json::Deserializer] Empty json object."};

                        if (!val.is_object())
                            throw std::invalid_argument{"[Mif::Remote::Serialization::Json::Deserializer] Json is no object type."};

                        m_value = val.as_object();
                    }

                    std::string const GetUuid() const
                    {
                        return m_value.at(Detail::Tag::Uuid::Value).as_string().c_str();
                    }

                    bool IsRequest() const
                    {
                        return GetType() == Detail::Tag::Request::Value;
                    }

                    bool IsResponse() const
                    {
                        return GetType() == Detail::Tag::Response::Value;
                    }

                    std::string const GetType() const
                    {
                        return m_value.at(Detail::Tag::Type::Value).as_string().c_str();
                    }

                    std::string const GetInstance() const
                    {
                        return m_value.at(Detail::Tag::Instsnce::Value).as_string().c_str();
                    }

                    std::string const GetInterface() const
                    {
                        return m_value.at(Detail::Tag::Interface::Value).as_string().c_str();
                    }

                    std::string const GetMethod() const
                    {
                        return m_value.at(Detail::Tag::Method::Value).as_string().c_str();
                    }

                    template <typename ... TParams>
                    std::tuple<typename std::decay<TParams>::type ... > GetParams() const
                    {
                        return GetParamsIfExists<TParams ... >();
                    }

                    bool HasException() const
                    {
                        return m_value.contains(Detail::Tag::Exception::Value);
                    }

                    std::exception_ptr GetException() const
                    {
                        std::exception_ptr ex{};

                        if (HasException())
                        {
                            try
                            {
                                std::string message{m_value.at(Detail::Tag::Exception::Value).as_string().c_str()};
                                throw std::runtime_error{std::move(message)};
                            }
                            catch (...)
                            {
                                ex = std::current_exception();
                            }
                        }

                        return ex;
                    }

                private:
                    boost::json::object m_value;

                    template <typename ... TParams>
                    typename std::enable_if<sizeof ... (TParams), std::tuple<typename std::decay<TParams>::type ... >>::type
                    GetParamsIfExists() const
                    {
                        using TResult = std::tuple<typename std::decay<TParams>::type ... >;
                        TResult res;
                        ::Mif::Serialization::Json::Detail::JsonToValue(m_value.at(Detail::Tag::Param::Value), res);
                        return res;
                    }

                    template <typename ... TParams>
                    typename std::enable_if< !sizeof ... (TParams), std::tuple<>>::type
                    GetParamsIfExists() const
                    {
                        return {};
                    }
                };

            }   // namespace Json
        }   // namespace Serialization
    }   //  namespace Remote
}   // namespace Mif


#endif  // !__MIF_REMOTE_SERIALIZATION_JSON_H__
