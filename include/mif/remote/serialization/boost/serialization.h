#ifndef __MIF_REMOTE_SERIALIZATION_BOOST_SERIALIZATION_H__
#define __MIF_REMOTE_SERIALIZATION_BOOST_SERIALIZATION_H__

// STD
#include <cstdint>
#include <string>
#include <vector>
#include <utility>

// BOOST
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/bitset.hpp>
#include <boost/serialization/boost_unordered_map.hpp>
#include <boost/serialization/boost_unordered_set.hpp>
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
                    using Buffer = std::vector<char>;

                    template <typename ... TParams>
                    Serializer(std::string const &instanceId, std::string const &interfaceId,
                        std::string const &methodId, TParams && ... params)
                    {
                        boost::iostreams::filtering_ostream stream(boost::iostreams::back_inserter(m_result));
                        TArchive archive(stream);
                        std::string type = Detail::Tag::Request;
                        archive << boost::serialization::make_nvp(Detail::Tag::Type, type);
                        archive << boost::serialization::make_nvp(Detail::Tag::Instsnce, instanceId);
                        archive << boost::serialization::make_nvp(Detail::Tag::Interface, interfaceId);
                        archive << boost::serialization::make_nvp(Detail::Tag::Method, methodId);
                        PutParams(archive, 1, std::forward<TParams>(params) ... );
                    }

                    Buffer  GetBuffer()
                    {
                        return std::move(m_result);
                    }

                private:
                    Buffer m_result;

                    template <typename TParam, typename ... TParams>
                    void PutParams(TArchive &archive, std::size_t index, TParam && param, TParams && ... params) const
                    {
                        archive << boost::serialization::make_nvp((Detail::Tag::Param + std::to_string(index)).c_str(), param);
                        PutParams(archive, index + 1, std::forward<TParams>(params) ... );
                    }

                    void PutParams(TArchive &, std::size_t) const
                    {
                    }
                };

                template <typename TArchive>
                class Deserializer final
                {
                public:
                    Deserializer()
                    {
                    }

                private:
                };

            }   // namespace Boost
        }   // namespace Serialization
    }   //  namespace Remote
}   // namespace Mif


#endif  // !__MIF_REMOTE_SERIALIZATION_BOOST_SERIALIZATION_H__
