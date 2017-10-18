//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_SERIALIZATION_JSON_H__
#define __MIF_REMOTE_SERIALIZATION_JSON_H__

// STD
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

// JSONCPP
#include <json/writer.h>

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
                        if (m_value.isMember(Detail::Tag::Param::Value))
                            m_value.removeMember(Detail::Tag::Param::Value);

                        auto const tuple = std::make_tuple(std::forward<TParams>(params) ... );
                        m_value[Detail::Tag::Param::Value] = ::Mif::Serialization::Json::Detail::ValueToJson(tuple);
                    }

                    void PutException(std::exception_ptr ex)
                    {
                        if (m_value.isMember(Detail::Tag::Exception::Value))
                            m_value.removeMember(Detail::Tag::Exception::Value);

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
                        ::Json::FastWriter writer;
                        auto data = writer.write(m_value);
                        return {std::begin(data), std::end(data)};
                    }

                private:
                    ::Json::Value m_value{::Json::objectValue};

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

                        ::Json::Reader reader;
                        if (!reader.parse({std::begin(buffer), std::end(buffer)}, m_value))
                            throw std::invalid_argument{"[Mif::Remote::Serialization::Json::Deserializer] Bad json."};

                        if (m_value.isNull())
                            throw std::invalid_argument{"[Mif::Remote::Serialization::Json::Deserializer] Empty json object."};

                        if (m_value.type() != ::Json::objectValue)
                            throw std::invalid_argument{"[Mif::Remote::Serialization::Json::Deserializer] Json is no object type."};
                    }

                    std::string const GetUuid() const
                    {
                        return m_value.get(Detail::Tag::Uuid::Value, "").asString();
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
                        return m_value.get(Detail::Tag::Type::Value, "").asString();
                    }

                    std::string const GetInstance() const
                    {
                        return m_value.get(Detail::Tag::Instsnce::Value, "").asString();
                    }

                    std::string const GetInterface() const
                    {
                        return m_value.get(Detail::Tag::Interface::Value, "").asString();
                    }

                    std::string const GetMethod() const
                    {
                        return m_value.get(Detail::Tag::Method::Value, "").asString();
                    }

                    template <typename ... TParams>
                    std::tuple<typename std::decay<TParams>::type ... > GetParams() const
                    {
                        return GetParamsIfExists<TParams ... >();
                    }

                    bool HasException() const
                    {
                        return m_value.isMember(Detail::Tag::Exception::Value);
                    }

                    std::exception_ptr GetException() const
                    {
                        std::exception_ptr ex{};

                        if (HasException())
                        {
                            try
                            {
                                auto message = m_value.get(Detail::Tag::Exception::Value, "").asString();
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
                    ::Json::Value m_value;

                    template <typename ... TParams>
                    typename std::enable_if<sizeof ... (TParams), std::tuple<typename std::decay<TParams>::type ... >>::type
                    GetParamsIfExists() const
                    {
                        using TResult = std::tuple<typename std::decay<TParams>::type ... >;
                        TResult res;
                        ::Mif::Serialization::Json::Detail::JsonToValue(m_value.get(Detail::Tag::Param::Value, ::Json::Value{}), res);
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
