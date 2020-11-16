//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERIALIZATION_BOOST_H__
#define __MIF_SERIALIZATION_BOOST_H__

// STD
#include <cstdint>
#include <string>
#include <tuple>

// BOOST
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
#include "mif/reflection/reflection.h"

namespace Mif
{
    namespace Serialization
    {
        namespace Boost
        {
            namespace Detail
            {

                template <std::size_t I>
                struct Serializer;

                template <typename TBases, std::size_t I = std::tuple_size<TBases>::value>
                struct BasesSerializer
                {
                    template <typename TArchive, typename T>
                    static void Serialize(TArchive &archive, T &object)
                    {
                        BasesSerializer<TBases, I - 1>::Serialize(archive, object);
                        using BaseType = typename std::tuple_element<I - 1, TBases>::type;
                        archive & boost::serialization::make_nvp(Reflection::Reflect<BaseType>::Name::Value,
                            boost::serialization::base_object<BaseType>(object));
                    }
                };

                template <typename TBases>
                struct BasesSerializer<TBases, 0>
                {
                    template <typename TArchive, typename T>
                    static void Serialize(TArchive &, T &)
                    {
                    }
                };

                template <std::size_t I>
                struct Serializer
                {
                    template <typename TArchive, typename T>
                    static void Serialize(TArchive &archive, T &object)
                    {
                        Serializer<I - 1>::Serialize(archive, object);
                        using FieldType = typename Reflection::Reflect<T>::Fields::template Field<I - 1>;
                        archive & boost::serialization::make_nvp(FieldType::Name::Value,
                            object.*FieldType::Access());
                    }
                };

                template <>
                struct Serializer<0>
                {
                    template <typename TArchive, typename T>
                    static void Serialize(TArchive &, T &)
                    {
                    }
                };

            }   // namespace Detail

            template <typename TArchive, typename T>
            inline void Serialize(TArchive &archive, T &object)
            {
                Detail::BasesSerializer<typename Reflection::Reflect<T>::Base>::Serialize(archive, object);
                Detail::Serializer<Reflection::Reflect<T>::Fields::Count>::Serialize(archive, object);
            }

        }   // namespace Boost
    }   // namespace Serialization
}   // namespace Mif

#undef MIF_BOOST_TYPE_SERIALIZER
#define MIF_BOOST_TYPE_SERIALIZER(type_) \
    namespace boost \
    { \
        namespace serialization \
        { \
            template<typename TArchive> \
            inline void serialize(TArchive &archive, type_ &object, unsigned int) \
            { \
                ::Mif::Serialization::Boost::Serialize(archive, object); \
            } \
        } \
    }

#endif  // !__MIF_SERIALIZATION_BOOST_H__
