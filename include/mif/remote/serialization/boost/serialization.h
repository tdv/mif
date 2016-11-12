//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_SERIALIZATION_BOOST_SERIALIZATION_H__
#define __MIF_REMOTE_SERIALIZATION_BOOST_SERIALIZATION_H__

// STD
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <tuple>
#include <utility>

// BOOST
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/bitset.hpp>
//#include <boost/serialization/boost_unordered_map.hpp>
//#include <boost/serialization/boost_unordered_set.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/queue.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/stack.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/weak_ptr.hpp>

// MIF
#include "mif/common/types.h"
#include "mif/serialization/boost.h"

namespace Mif
{
    namespace Remote
    {
        namespace Serialization
        {
            namespace Boost
            {
                namespace Detail
                {

                    struct Tag
                    {
                        static constexpr auto Uuid = "uuid";
                        static constexpr auto Type = "type";
                        static constexpr auto Request = "request";
                        static constexpr auto Response = "response";
                        static constexpr auto Instsnce = "instance";
                        static constexpr auto Interface = "interface";
                        static constexpr auto Method = "method";
                        static constexpr auto Param = "prm";
                    };

                }   // namespace Detail

                template <typename TArchive>
                class Serializer final
                {
                public:
                    template <typename ... TParams>
                    Serializer(bool isReques, std::string const &uuid,
                        std::string const &instanceId, std::string const &interfaceId,
                        std::string const &methodId, TParams && ... params)
                        : m_stream(boost::iostreams::back_inserter(m_result))
                        , m_archive(new TArchive(m_stream))
                    {
                        *m_archive << boost::serialization::make_nvp(Detail::Tag::Uuid, uuid);
                        std::string type = isReques ? Detail::Tag::Request : Detail::Tag::Response;
                        *m_archive << boost::serialization::make_nvp(Detail::Tag::Type, type);
                        *m_archive << boost::serialization::make_nvp(Detail::Tag::Instsnce, instanceId);
                        *m_archive << boost::serialization::make_nvp(Detail::Tag::Interface, interfaceId);
                        *m_archive << boost::serialization::make_nvp(Detail::Tag::Method, methodId);
                        SaveParams(1, std::forward<TParams>(params) ... );
                    }

                    template <typename ... TParams>
                    void PutParams(TParams && ... params)
                    {
                        if (!m_archive)
                        {
                            throw std::runtime_error{"[Mif::Remote::Serialization::Boost::Serializar::PutParams] "
                                "Failed to put params. Archive closed."};
                        }
                        SaveParams(1, std::forward<TParams>(params) ... );
                    }

                    Common::Buffer GetBuffer()
                    {
                        if (!m_archive)
                        {
                            throw std::runtime_error{"[Mif::Remote::Serialization::Boost::Serializar::GetBuffer] "
                                "Failed to get buffer. Archive closed."};
                        }
                        m_archive.reset();
                        m_stream.flush();;
                        return std::move(m_result);
                    }

                private:
                    std::vector<char> m_result;
                    boost::iostreams::filtering_ostream m_stream;
                    std::unique_ptr<TArchive> m_archive;

                    template <typename TParam, typename ... TParams>
                    void SaveParams(std::size_t index, TParam && param, TParams && ... params)
                    {
                        *m_archive << boost::serialization::make_nvp((Detail::Tag::Param + std::to_string(index)).c_str(), param);
                        SaveParams(index + 1, std::forward<TParams>(params) ... );
                    }

                    void SaveParams(std::size_t)
                    {
                    }
                };

                template <typename TArchive>
                class Deserializer final
                {
                public:
                    Deserializer(Common::Buffer buffer)
                        : m_buffer(std::move(buffer))
                        , m_source(!m_buffer.empty() ? m_buffer.data() : nullptr, m_buffer.size())
                        , m_stream(m_source)
                        , m_archive(m_stream)
                    {
                        if (m_buffer.empty())
                            throw std::invalid_argument{"[Mif::Remote::Serialization::Boost::Deserializer] Empty buffer."};
                        m_archive >> boost::serialization::make_nvp(Detail::Tag::Uuid, m_uuid);
                        m_archive >> boost::serialization::make_nvp(Detail::Tag::Type, m_type);
                        m_archive >> boost::serialization::make_nvp(Detail::Tag::Instsnce, m_instance);
                        m_archive >> boost::serialization::make_nvp(Detail::Tag::Interface, m_interface);
                        m_archive >> boost::serialization::make_nvp(Detail::Tag::Method, m_method);
                    }

                    std::string const& GetUuid() const
                    {
                        return m_uuid;
                    }

                    bool IsRequest() const
                    {
                        return GetType() == Detail::Tag::Request;
                    }

                    bool IsResponse() const
                    {
                        return GetType() == Detail::Tag::Response;
                    }

                    std::string const& GetType() const
                    {
                        return m_type;
                    }

                    std::string const& GetInstance() const
                    {
                        return m_instance;
                    }

                    std::string const& GetInterface() const
                    {
                        return m_interface;
                    }

                    std::string const& GetMethod() const
                    {
                        return m_method;
                    }

                    template <typename ... TParams>
                    std::tuple<typename std::decay<TParams>::type ... > GetParams() const
                    {
                        using TResult = std::tuple<typename std::decay<TParams>::type ... >;
                        TResult res;
                        LoadParams(reinterpret_cast
                                    <
                                        std::integral_constant
                                            <
                                                std::size_t, std::tuple_size<TResult>::value
                                            > const *
                                    >(0), res);
                        return res;
                    }

                private:
                    using SourceType = boost::iostreams::basic_array_source<char>;
                    Common::Buffer m_buffer;
                    SourceType m_source;
                    boost::iostreams::stream<SourceType> m_stream;
                    mutable TArchive m_archive;
                    std::string m_uuid;
                    std::string m_type;
                    std::string m_instance;
                    std::string m_interface;
                    std::string m_method;

                    template <std::size_t Index, typename TParams,
                              typename = typename std::enable_if<Index>::type>
                    void LoadParams(std::integral_constant<std::size_t, Index> const *,
                                   TParams &params) const
                    {
                        auto &param = std::get<std::tuple_size<TParams>::value - Index>(params);
                        m_archive >> boost::serialization::make_nvp((Detail::Tag::Param + std::to_string(Index)).c_str(), param);
                        LoadParams(reinterpret_cast
                                    <
                                        std::integral_constant
                                            <
                                                std::size_t, Index - 1
                                            > const *
                                    >(0), params);
                    }

                    template <typename TParams>
                    void LoadParams(std::integral_constant<std::size_t, 0> const *, TParams &) const
                    {
                    }
                };

            }   // namespace Boost
        }   // namespace Serialization
    }   //  namespace Remote
}   // namespace Mif


#endif  // !__MIF_REMOTE_SERIALIZATION_BOOST_SERIALIZATION_H__
