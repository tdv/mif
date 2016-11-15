//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_SERIALIZATION_BOOST_H__
#define __MIF_REMOTE_SERIALIZATION_BOOST_H__

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
//#include <boost/serialization/bitset.hpp>
////#include <boost/serialization/boost_unordered_map.hpp>
////#include <boost/serialization/boost_unordered_set.hpp>
//#include <boost/serialization/deque.hpp>
//#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
//#include <boost/serialization/optional.hpp>
//#include <boost/serialization/queue.hpp>
//#include <boost/serialization/scoped_ptr.hpp>
//#include <boost/serialization/set.hpp>
//#include <boost/serialization/shared_ptr.hpp>
//#include <boost/serialization/stack.hpp>
#include <boost/serialization/string.hpp>
//#include <boost/serialization/unique_ptr.hpp>
//#include <boost/serialization/unordered_map.hpp>
//#include <boost/serialization/unordered_set.hpp>
//#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
//#include <boost/serialization/weak_ptr.hpp>

// MIF
#include "mif/common/types.h"
#include "mif/common/unused.h"
#include "mif/remote/serialization/detail/tag.h"
#include "mif/serialization/boost.h"

namespace Mif
{
    namespace Remote
    {
        namespace Serialization
        {
            namespace Detail
            {
                namespace Tag
                {

                    MIF_DECLARE_SRTING_PROVIDER(HasException, "has_exception")

                }   // namespace Tag
            }   // namespace Detail

            namespace Boost
            {

                template <typename TArchive>
                class Serializer final
                {
                public:
                    template <typename ... TParams>
                    Serializer(bool isReques, std::string const &uuid,
                        std::string const &instanceId, std::string const &interfaceId,
                        std::string const &methodId, TParams && ... params)
                        : m_type{isReques ? Detail::Tag::Request::GetString() : Detail::Tag::Response::GetString()}
                        , m_uuid{uuid}
                        , m_instanceId{instanceId}
                        , m_interfaceId{interfaceId}
                        , m_methodId{methodId}
                        , m_params{new ParamPack<TParams ... >{std::forward<TParams>(params) ... }}
                    {
                    }

                    template <typename ... TParams>
                    void PutParams(TParams && ... params)
                    {
                        m_params.reset(new ParamPack<TParams ... >{std::forward<TParams>(params) ... });
                    }

                    void PutException(std::exception_ptr ex)
                    {
                        m_exception = ex;
                    }

                    Common::Buffer GetBuffer()
                    {
                        Common::Buffer result;

                        {
                            boost::iostreams::filtering_ostream stream(boost::iostreams::back_inserter(result));
                            TArchive archive{stream};

                            archive << boost::serialization::make_nvp(Detail::Tag::Uuid::GetString(), m_uuid);
                            archive << boost::serialization::make_nvp(Detail::Tag::Type::GetString(), m_type);
                            archive << boost::serialization::make_nvp(Detail::Tag::Instsnce::GetString(), m_instanceId);
                            archive << boost::serialization::make_nvp(Detail::Tag::Interface::GetString(), m_interfaceId);
                            archive << boost::serialization::make_nvp(Detail::Tag::Method::GetString(), m_methodId);

                            bool hasException = !!m_exception;
                            archive << boost::serialization::make_nvp(Detail::Tag::HasException::GetString(), hasException);

                            if (hasException)
                            {
                                std::string message;
                                try
                                {
                                    std::rethrow_exception(m_exception);
                                }
                                catch (std::exception const &e)
                                {
                                    message = e.what();
                                }
                                catch (...)
                                {
                                    message = "Unknown exception.";
                                }
                                archive << boost::serialization::make_nvp(Detail::Tag::Exception::GetString(), message);
                            }

                            m_params->Save(archive);

                            stream.flush();
                        }

                        return result;
                    }

                private:
                    std::string m_type;
                    std::string m_uuid;
                    std::string m_instanceId;
                    std::string m_interfaceId;
                    std::string m_methodId;
                    std::exception_ptr m_exception{};

                    struct IData
                    {
                        virtual ~IData() = default;
                        virtual void Save(TArchive &archive) = 0;
                    };

                    std::unique_ptr<IData> m_params;

                    template <typename ... TParams>
                    class ParamPack final
                        : public IData
                    {
                    public:
                        ParamPack(TParams && ... params)
                            : m_params{std::forward<TParams>(params) ... }
                        {
                        }

                    private:
                        std::tuple<typename std::decay<TParams>::type ... > m_params;
                        // IData
                        virtual void Save(TArchive &archive) override final
                        {
                            SaveParams(m_params, archive);
                        }

                        template <typename T, std::size_t ... Indexes>
                        void SaveTupleParams(T &params, TArchive &archive, Common::IndexSequence<Indexes ... > const *) const
                        {
                            Common::Unused(archive << boost::serialization::make_nvp((Detail::Tag::Param::GetString() +
                                std::to_string(Indexes)).c_str(), std::get<Indexes>(params)) ... );
                        }

                        template <typename ... T>
                        typename std::enable_if<std::tuple_size<std::tuple<T ... >>::value, void>::type
                        SaveParams(std::tuple<T ... > &params, TArchive &archive) const
                        {
                            SaveTupleParams(params, archive,
                                static_cast<Common::MakeIndexSequence<sizeof ... (T)> const *>(nullptr));
                        }

                        template <typename ... T>
                        typename std::enable_if< !std::tuple_size<std::tuple<T ... >>::value, void>::type
                        SaveParams(std::tuple<T ... > &, TArchive &) const
                        {
                        }
                    };
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
                        m_archive >> boost::serialization::make_nvp(Detail::Tag::Uuid::GetString(), m_uuid);
                        m_archive >> boost::serialization::make_nvp(Detail::Tag::Type::GetString(), m_type);
                        m_archive >> boost::serialization::make_nvp(Detail::Tag::Instsnce::GetString(), m_instance);
                        m_archive >> boost::serialization::make_nvp(Detail::Tag::Interface::GetString(), m_interface);
                        m_archive >> boost::serialization::make_nvp(Detail::Tag::Method::GetString(), m_method);

                        bool hasException = false;
                        m_archive >> boost::serialization::make_nvp(Detail::Tag::HasException::GetString(), hasException);
                        if (hasException)
                        {
                            std::string message;
                            m_archive >> boost::serialization::make_nvp(Detail::Tag::Exception::GetString(), message);
                            try
                            {
                                throw std::runtime_error{message};
                            }
                            catch (...)
                            {
                                m_exception = std::current_exception();
                            }
                        }
                    }

                    std::string const& GetUuid() const
                    {
                        return m_uuid;
                    }

                    bool IsRequest() const
                    {
                        return GetType() == Detail::Tag::Request::GetString();
                    }

                    bool IsResponse() const
                    {
                        return GetType() == Detail::Tag::Response::GetString();
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

                    bool HasException() const
                    {
                        return !!m_exception;
                    }

                    std::exception_ptr GetException() const
                    {
                        return m_exception;
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
                    std::exception_ptr m_exception;

                    template <std::size_t Index, typename TParams,
                              typename = typename std::enable_if<Index>::type>
                    void LoadParams(std::integral_constant<std::size_t, Index> const *,
                                   TParams &params) const
                    {
                        auto &param = std::get<Index - 1>(params);
                        m_archive >> boost::serialization::make_nvp((Detail::Tag::Param::GetString() + std::to_string(Index)).c_str(), param);
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


#endif  // !__MIF_REMOTE_SERIALIZATION_BOOST_H__
