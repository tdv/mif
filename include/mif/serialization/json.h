//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERIALIZATION_JSON_H__
#define __MIF_SERIALIZATION_JSON_H__

// STD
#include <array>
#include <stdexcept>
#include <cstdint>
#include <iomanip>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

// BOOST
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/json.hpp>

// MIF
#include "mif/common/config.h"
#include "mif/common/types.h"
#include "mif/common/index_sequence.h"
#include "mif/common/static_string.h"
#include "mif/common/unused.h"
#include "mif/reflection/reflection.h"
#include "mif/serialization/traits.h"

// Use MIF_PRETTY_JSON_WRITER define in order to write pretty json

namespace Mif
{
    namespace Serialization
    {
        namespace Json
        {
            namespace Detail
            {
                namespace Tag
                {

                    using Id = MIF_STATIC_STR("id");
                    using Value = MIF_STATIC_STR("val");

                }   // namespace Tag

                template <typename, std::size_t>
                struct BasesSerializer;

                template <std::size_t>
                struct Serializer;

                template <std::size_t>
                struct Deserializer;

                template <typename, std::size_t>
                struct BasesDeserializer;

                template <typename T>
                typename std::enable_if<!Reflection::IsReflectable<T>() && std::is_enum<T>::value, boost::json::value>::type
                ValueToJson(T const &object);

                template <typename T>
                typename std::enable_if<Reflection::IsReflectable<T>() && std::is_enum<T>::value, boost::json::value>::type
                ValueToJson(T const &object);

                template <typename T>
                typename std::enable_if<Traits::IsSimple<T>(), boost::json::value>::type
                ValueToJson(T const &object);

                template <typename T>
                typename std::enable_if<Reflection::IsReflectable<T>() && !std::is_enum<T>::value, boost::json::value>::type
                ValueToJson(T const &object);

                template <typename T>
                typename std::enable_if<Traits::IsSmartPointer<T>(), boost::json::value>::type
                ValueToJson(T const &ptr);

                template <typename T>
                typename std::enable_if<Traits::IsOptional<T>(), boost::json::value>::type
                ValueToJson(T const &value);

                template <typename T>
                typename std::enable_if
                    <
                        Traits::IsIterable<T>() &&
                            !std::is_same<typename std::decay<typename T::value_type>::type, char>::value,
                        boost::json::value
                    >::type
                ValueToJson(T const &array);

                template <typename T>
                typename std::enable_if
                    <
                        Traits::IsIterable<T>() &&
                            std::is_same<typename std::decay<typename T::value_type>::type, char>::value,
                        boost::json::value
                    >::type
                ValueToJson(T const &array);

                template <typename ... T>
                boost::json::value ValueToJson(std::tuple<T ... > const &tuple);

                template <typename TFirst, typename TSecond>
                boost::json::value ValueToJson(std::pair<TFirst, TSecond> const &pair);

                template <typename T>
                typename std::enable_if<Traits::IsSimple<T>(), T>::type&
                JsonToValue(boost::json::value const &root, T &object);

                template <typename T>
                inline typename std::enable_if<!Reflection::IsReflectable<T>() && std::is_enum<T>::value, T>::type&
                JsonToValue(boost::json::value const &root, T &object);

                template <typename T>
                inline typename std::enable_if<Reflection::IsReflectable<T>() && std::is_enum<T>::value, T>::type&
                JsonToValue(boost::json::value const &root, T &object);

                template <typename T>
                typename std::enable_if<Reflection::IsReflectable<T>() && !std::is_enum<T>::value, T>::type&
                JsonToValue(boost::json::value const &root, T &object);

                template <typename T>
                typename std::enable_if<Traits::IsSmartPointer<T>(), T>::type&
                JsonToValue(boost::json::value const &root, T &object);

                template <typename T>
                typename std::enable_if<Traits::IsOptional<T>(), T>::type&
                JsonToValue(boost::json::value const &root, T &object);

                template <typename TFirst, typename TSecond>
                std::pair<TFirst, TSecond>&
                JsonToValue(boost::json::value const &root, std::pair<TFirst, TSecond> &pair);

                template <typename ... T>
                std::tuple<T ... >&
                JsonToValue(boost::json::value const &root, std::tuple<T ... > &tuple);

                template <typename T, std::size_t N>
                std::array<T, N>&
                JsonToValue(boost::json::value const &root, std::array<T, N> &array);

                template <typename T>
                inline typename std::enable_if
                    <
                        Traits::IsIterable<T>() &&
                            !std::is_same<typename std::decay<typename T::value_type>::type, char>::value,
                        T
                    >::type&
                JsonToValue(boost::json::value const &root, T &object);

                template <typename T>
                inline typename std::enable_if
                    <
                        Traits::IsIterable<T>() &&
                            std::is_same<typename std::decay<typename T::value_type>::type, char>::value,
                        T
                    >::type&
                JsonToValue(boost::json::value const &root, T &object);

                template <typename T>
                inline typename std::enable_if<std::is_pointer<T>::value, boost::json::value>::type
                ValueToJson(T const &)
                {
                    static_assert(!std::is_pointer<T>::value, "[Mif::Serialization::Json::Detail] You can't serialize the raw pointers.");
                }

                template <typename T>
                inline typename std::enable_if<!Reflection::IsReflectable<T>() && std::is_enum<T>::value, boost::json::value>::type
                ValueToJson(T const &object)
                {
                    return ValueToJson(static_cast<typename std::underlying_type<T>::type>(object));
                }

                template <typename T>
                inline typename std::enable_if<Reflection::IsReflectable<T>() && std::is_enum<T>::value, boost::json::value>::type
                ValueToJson(T const &object)
                {
                    return  boost::json::value_from(Reflection::ToString(object));
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsSimple<T>(), boost::json::value>::type
                ValueToJson(T const &object)
                {
                    return boost::json::value_from(object);
                }

                template <typename T>
                inline typename std::enable_if<Reflection::IsReflectable<T>() && !std::is_enum<T>::value, boost::json::value>::type
                ValueToJson(T const &object)
                {
                    auto root = boost::json::value_from(boost::json::object{});
                    using BasesType = typename Reflection::Reflect<T>::Base;
                    BasesSerializer<BasesType, std::tuple_size<BasesType>::value>::Serialize(root, object);
                    Serializer<Reflection::Reflect<T>::Fields::Count>::Serialize(root, object);
                    return boost::json::value_from(root);
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsSmartPointer<T>(), boost::json::value>::type
                ValueToJson(T const &ptr)
                {
                    if (!ptr)
                        return boost::json::value{};
                    return ValueToJson(*ptr);
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsOptional<T>(), boost::json::value>::type
                ValueToJson(T const &value)
                {
                    if (!value)
                        return boost::json::value{};
                    return ValueToJson(*value);
                }

                template <typename T>
                inline typename std::enable_if
                    <
                        Traits::IsIterable<T>() &&
                            !std::is_same<typename std::decay<typename T::value_type>::type, char>::value,
                        boost::json::value
                    >::type
                ValueToJson(T const &array)
                {
                    boost::json::array root;

                    for (auto const &i : array)
                        root.push_back(ValueToJson(i));

                    return boost::json::value_from(root);
                }

                template <typename T>
                inline typename std::enable_if
                    <
                        Traits::IsIterable<T>() &&
                            std::is_same<typename std::decay<typename T::value_type>::type, char>::value,
                        boost::json::value
                    >::type
                ValueToJson(T const &array)
                {
                    boost::json::array root;

                    std::ostringstream stream;

                    for (auto const &i : array)
                    {
                        stream << std::hex << std::setw(2) << std::setfill('0')
                               << static_cast<int>(i) << ' ';
                    }

                    root.push_back(ValueToJson(stream.str()));

                    return boost::json::value_from(root);
                }

                template <typename T, std::size_t ... Indexes>
                inline boost::json::value TupleToJson(T const &tuple, Common::IndexSequence<Indexes ... > const *)
                {
                    boost::json::array root;
                    root.resize(sizeof ... (Indexes));

                    Common::Unused(root[Indexes] = ValueToJson(std::get<Indexes>(tuple)) ... );

                    return boost::json::value_from(root);
                }

                template <typename ... T>
                inline boost::json::value ValueToJson(std::tuple<T ... > const &tuple)
                {
                    using TupleType = std::tuple<T ... >;
                    return TupleToJson(tuple, static_cast<Common::MakeIndexSequence<std::tuple_size<TupleType>::value> const *>(nullptr));
                }

                template <typename TFirst, typename TSecond>
                inline boost::json::value ValueToJson(std::pair<TFirst, TSecond> const &pair)
                {
                    boost::json::object root;

                    root[Tag::Id::Value] = ValueToJson(pair.first);
                    root[Tag::Value::Value] = ValueToJson(pair.second);

                    return boost::json::value_from(root);
                }

                template <typename TBases, std::size_t I>
                struct BasesSerializer
                {
                    template <typename T>
                    static void Serialize(boost::json::value &root, T const &object)
                    {
                        BasesSerializer<TBases, I - 1>::Serialize(root, object);
                        using BaseType = typename std::tuple_element<I - 1, TBases>::type;
                        root.as_object()[Reflection::Reflect<BaseType>::Name::Value] = ValueToJson(static_cast<BaseType const &>(object));
                    }
                };

                template <typename TBases>
                struct BasesSerializer<TBases, 0>
                {
                    template <typename T>
                    static void Serialize(boost::json::value &, T const &)
                    {
                    }
                };

                template <std::size_t I>
                struct Serializer
                {
                    template <typename T>
                    static void Serialize(boost::json::value &root, T const &object)
                    {
                        Serializer<I - 1>::Serialize(root, object);
                        using FieldType = typename Reflection::Reflect<T>::Fields::template Field<I - 1>;
                        root.as_object()[FieldType::Name::Value] = ValueToJson(object.*FieldType::Access());
                    }
                };

                template <>
                struct Serializer<0>
                {
                    template <typename T>
                    static void Serialize(boost::json::value &, T &)
                    {
                    }
                };

                template <typename T>
                struct JsonValueConverter;

                template <>
                struct JsonValueConverter<std::string>
                {
                    static std::string Convert(boost::json::value const &val)
                    {
                        if (auto const *v = val.if_bool())
                            return std::to_string(*v);
                        if (auto const *v = val.if_double())
                            return std::to_string(*v);
                        if (auto const *v = val.if_int64())
                            return std::to_string(*v);
                        if (auto const *v = val.if_string())
                            return v->c_str();
                        if (auto const *v = val.if_uint64())
                            return std::to_string(*v);

                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to string."};
                    }
                };

                template <>
                struct JsonValueConverter<double>
                {
                    static double Convert(boost::json::value const &val)
                    {
                        if (auto const *v = val.if_double())
                            return *v;
                        if (auto const *v = val.if_int64())
                            return *v;
                        if (auto const *v = val.if_uint64())
                            return *v;

                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to double."};
                    }
                };

                template <>
                struct JsonValueConverter<bool>
                {
                    static bool Convert(boost::json::value const &val)
                    {
                        if (auto const *v = val.if_bool())
                            return *v;
                        if (auto const *v = val.if_int64())
                            return !!*v;
                        if (auto const *v = val.if_uint64())
                            return !!*v;

                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to bool."};
                    }
                };

                template <>
                struct JsonValueConverter<std::int64_t>
                {
                    static std::int64_t Convert(boost::json::value const &val)
                    {
                        if (auto const *v = val.if_int64())
                            return *v;

                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to int64."};
                    }
                };

                template <>
                struct JsonValueConverter<std::uint64_t>
                {
                    static std::uint64_t Convert(boost::json::value const &val)
                    {
                        if (auto const *v = val.if_uint64())
                            return *v;

                        if (auto const *v = val.if_int64())
                            return static_cast<std::uint64_t>(*v);

                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to uint64."};
                    }
                };

                template <>
                struct JsonValueConverter<std::int32_t>
                {
                    static std::int32_t Convert(boost::json::value const &val)
                    {
                        if (auto const *v = val.if_int64())
                            return static_cast<std::int32_t>(*v);

                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to int32."};
                    }
                };

                template <>
                struct JsonValueConverter<std::uint32_t>
                {
                    static std::uint32_t Convert(boost::json::value const &val)
                    {
                        if (auto const *v = val.if_uint64())
                            return static_cast<std::uint32_t>(*v);

                        if (auto const *v = val.if_int64())
                            return static_cast<std::uint32_t>(*v);

                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to uint32."};
                    }
                };

                template <>
                struct JsonValueConverter<std::int16_t>
                {
                    static std::int16_t Convert(boost::json::value const &val)
                    {
                        return static_cast<std::int16_t>(JsonValueConverter<std::int32_t>::Convert(val));
                    }
                };

                template <>
                struct JsonValueConverter<std::uint16_t>
                {
                    static std::uint16_t Convert(boost::json::value const &val)
                    {
                        return static_cast<std::uint16_t>(JsonValueConverter<std::int32_t>::Convert(val));
                    }
                };

                template <>
                struct JsonValueConverter<std::int8_t>
                {
                    static std::int8_t Convert(boost::json::value const &val)
                    {
                        return static_cast<std::int8_t>(JsonValueConverter<std::int32_t>::Convert(val));
                    }
                };

                template <>
                struct JsonValueConverter<std::uint8_t>
                {
                    static std::uint8_t Convert(boost::json::value const &val)
                    {
                        return static_cast<std::uint8_t>(JsonValueConverter<std::int32_t>::Convert(val));
                    }
                };

                template <typename T>
                inline typename std::enable_if<Traits::IsSimple<T>(), T>::type&
                JsonToValue(boost::json::value const &root, T &object)
                {
                    if (root.is_null())
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to get value from null."};
                    object = JsonValueConverter<T>::Convert(root);
                    return object;
                }

                template <typename T>
                inline typename std::enable_if<!Reflection::IsReflectable<T>() && std::is_enum<T>::value, T>::type&
                JsonToValue(boost::json::value const &root, T &object)
                {
                    if (root.is_null())
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to get value from null."};
                    object = static_cast<T>(JsonValueConverter<typename std::underlying_type<T>::type>::Convert(root));
                    return object;
                }

                template <typename T>
                inline typename std::enable_if<Reflection::IsReflectable<T>() && std::is_enum<T>::value, T>::type&
                JsonToValue(boost::json::value const &root, T &object)
                {
                    if (root.is_null())
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to get value from null."};
                    object = Reflection::FromString<T>(JsonValueConverter<std::string>::Convert(root));
                    return object;
                }

                template <typename T>
                inline typename std::enable_if<Reflection::IsReflectable<T>() && !std::is_enum<T>::value, T>::type&
                JsonToValue(boost::json::value const &root, T &object)
                {
                    auto const *val = root.if_object();
                    if (!val)
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to get value. "
                            "Json element is invalid or is not converted to the correct type."};
                    }

                    using BasesType = typename Reflection::Reflect<T>::Base;
                    BasesDeserializer<BasesType, std::tuple_size<BasesType>::value>::Deserialize(root, object);

                    Deserializer<Reflection::Reflect<T>::Fields::Count>::Deserialize(*val, object);
                    return object;
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsSmartPointer<T>(), T>::type&
                JsonToValue(boost::json::value const &root, T &object)
                {
                    if (root.is_null())
                        return object;

                    using ObjectType = typename T::element_type;
                    object.reset(new ObjectType{});
                    JsonToValue<ObjectType>(root, *object);

                    return object;
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsOptional<T>(), T>::type&
                JsonToValue(boost::json::value const &root, T &object)
                {
                    if (root.is_null())
                        return object;

                    using ObjectType = typename T::value_type;
                    ObjectType value;
                    JsonToValue<ObjectType>(root, value);
                    object.reset(value);

                    return object;
                }

                template <typename TFirst, typename TSecond>
                inline std::pair<TFirst, TSecond>&
                JsonToValue(boost::json::value const &root, std::pair<TFirst, TSecond> &pair)
                {
                    auto const *obj = root.if_object();
                    if (!obj && !obj->contains(Tag::Id::Value) && !obj->contains(Tag::Value::Value))
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to parse pair. "
                            "Value is null or json has no pair type object."};
                    }

                    JsonToValue(*obj->if_contains(Tag::Id::Value),
                            const_cast<typename std::remove_const<TFirst>::type &>(pair.first));

                    JsonToValue(*obj->if_contains(Tag::Value::Value), pair.second);

                    return pair;
                }

                template <typename T, std::size_t ... Indexes>
                inline void JsonToTuple(boost::json::value const &root, T &tuple, Common::IndexSequence<Indexes ... > const *)
                {
                    Common::Unused(JsonToValue(root.as_array()[Indexes], std::get<Indexes>(tuple)) ... );
                }

                template <typename ... T>
                inline std::tuple<T ... >&
                JsonToValue(boost::json::value const &root, std::tuple<T ... > &tuple)
                {
                    auto const *array = root.if_array();
                    if (!array)
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to parse pair. "
                            "Value is null or json has no tuple type object."};
                    }

                    using TupleType = std::tuple<T ... >;
                    JsonToTuple(*array, tuple, static_cast<Common::MakeIndexSequence<std::tuple_size<TupleType>::value> const *>(nullptr));

                    return tuple;
                }

                template <typename T, std::size_t ... Indexes>
                inline void JsonToArray(boost::json::value const &root, std::array<T, sizeof ... (Indexes)> &array, Common::IndexSequence<Indexes ... > const *)
                {
                    Common::Unused(JsonToValue(root.as_array()[Indexes], array[Indexes]) ... );
                }

                template <typename T, std::size_t N>
                inline std::array<T, N>&
                JsonToValue(boost::json::value const &root, std::array<T, N> &array)
                {
                    auto const *arr = root.if_array();
                    if (!arr)
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to parse pair. "
                            "Value is null or json has no array type object."};
                    }

                    std::array<T, N> tmp;
                    JsonToArray(*arr, tmp, static_cast<Common::MakeIndexSequence<N> const *>(nullptr));
                    std::swap(array, tmp);

                    return array;
                }

                template <typename T>
                inline typename std::enable_if
                    <
                        Traits::IsIterable<T>() &&
                            !std::is_same<typename std::decay<typename T::value_type>::type, char>::value,
                        T
                    >::type&
                JsonToValue(boost::json::value const &root, T &object)
                {
                    auto const *arr = root.if_array();
                    if (!arr)
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to get value. "
                            "Json element is invalid or is not converted to the array."};
                    }

                    T{}.swap(object);

                    if (!arr->size())
                        return object;

                    using ObjectType = typename T::value_type;

                    for (auto const &i : *arr)
                    {
                        ObjectType item;
                        JsonToValue(i, item);
                        *std::inserter(object, std::end(object)) = std::move(item);
                    }

                    return object;
                }

                template <typename T>
                inline typename std::enable_if
                    <
                        Traits::IsIterable<T>() &&
                            std::is_same<typename std::decay<typename T::value_type>::type, char>::value,
                        T
                    >::type&
                JsonToValue(boost::json::value const &root, T &object)
                {
                    auto const *arr = root.if_string();
                    if (!arr)
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to get value. "
                            "Json element is invalid or is not converted to the array."};
                    }

                    T{}.swap(object);

                    if (!arr->size())
                        return object;

                    using ObjectType = typename T::value_type;

                    std::stringstream stream{arr->c_str()};

                    for (int i = 0 ; stream.good() ; i = 0)
                    {
                        stream >> std::hex >> std::setw(2) >> std::setfill('0') >> i;
                        ObjectType item{static_cast<ObjectType>(i)};
                        *std::inserter(object, std::end(object)) = std::move(item);
                    }

                    return object;
                }

                template <typename TBases, std::size_t I>
                struct BasesDeserializer
                {
                    template <typename T>
                    static void Deserialize(boost::json::value const &root, T &object)
                    {
                        BasesDeserializer<TBases, I - 1>::Deserialize(root, object);
                        using BaseType = typename std::tuple_element<I - 1, TBases>::type;
                        auto tmp = boost::json::value_from(root.as_object().at(Reflection::Reflect<BaseType>::Name::Value));
                        JsonToValue(tmp, static_cast<BaseType &>(object));
                    }
                };

                template <typename TBases>
                struct BasesDeserializer<TBases, 0>
                {
                    template <typename T>
                    static void Deserialize(boost::json::value const &, T &)
                    {
                    }
                };


                template <std::size_t I>
                struct Deserializer
                {
                    template <typename T>
                    static void Deserialize(boost::json::value const &root, T &object)
                    {
                        Deserializer<I - 1>::Deserialize(root, object);
                        using FieldType = typename Reflection::Reflect<T>::Fields::template Field<I - 1>;
                        auto const &item = root.as_object();
                        JsonToValue(item.contains(FieldType::Name::Value) ?
                                item.at(FieldType::Name::Value) :
                                boost::json::value{},
                                object.*FieldType::Access());
                    }
                };

                template <>
                struct Deserializer<0>
                {
                    template <typename T>
                    static void Deserialize(boost::json::value const &, T &)
                    {
                    }
                };

#ifdef MIF_PRETTY_JSON_WRITER
                inline void Write(std::ostream &os, boost::json::value const &val,
                        std::size_t level = 0)
                {
                    static constexpr std::size_t indentSize = 4;

                    std::string indent;
                    switch(val.kind())
                    {
                    case boost::json::kind::object :
                    {
                        ++level;
                        indent.append(level * indentSize, ' ');
                        os << "{\n";
                        auto const &obj = val.get_object();
                        if (!obj.empty())
                        {
                            for(auto i = std::begin(obj) ; i != std::end(obj) ; )
                            {
                                os << indent << boost::json::serialize(i->key()) << ": ";
                                Write(os, i->value(), level);
                                if(++i == std::end(obj))
                                    break;
                                os << ",\n";
                            }
                        }
                        os << "\n";
                        --level;
                        indent.resize(level * indentSize);
                        os << indent << "}";
                        break;
                    }
                    case boost::json::kind::array :
                    {
                        os << "[\n";
                        ++level;
                        indent.append(level * indentSize, ' ');
                        auto const &arr = val.get_array();
                        for(auto i = std::begin(arr) ; i != std::end(arr) ; )
                        {
                            os << indent;
                            Write(os, *i, level);
                            if(++i == std::end(arr))
                                break;
                            os << ",\n";
                        }
                        os << "\n";
                        --level;
                        indent.resize(level * indentSize);
                        os << indent << "]";
                        break;
                    }
                    case boost::json::kind::string :
                    {
                        os << boost::json::serialize(val.get_string());
                        break;
                    }
                    case boost::json::kind::uint64 :
                        os << val.get_uint64();
                        break;
                    case boost::json::kind::int64 :
                        os << val.get_int64();
                        break;
                    case boost::json::kind::double_ :
                        os << val.get_double();
                        break;
                    case boost::json::kind::bool_ :
                        os << (val.get_bool() ? "true" : "false");
                        break;
                    case boost::json::kind::null :
                        os << "null";
                        break;
                    }
                    if(!level)
                        os << "\n";
                }
#else
                inline void Write(std::ostream &os, boost::json::value const &val)
                {
                    os << val;
                }
#endif  // !MIF_PRETTY_JSON_WRITER

            }   // namespace Detail

            template <typename T, typename TStream>
            inline typename std::enable_if<Reflection::IsReflectable<T>(), void>::type
            Serialize(T const &object, TStream &stream)
            {
                auto root = boost::json::value_from(boost::json::object{});
                using BasesType = typename Reflection::Reflect<T>::Base;
                Detail::BasesSerializer<BasesType, std::tuple_size<BasesType>::value>::Serialize(root, object);
                Detail::Serializer<Reflection::Reflect<T>::Fields::Count>::Serialize(root, object);
                Detail::Write(stream, root);
            }

            template <typename T>
            inline typename std::enable_if<Reflection::IsReflectable<T>(), Common::Buffer>::type
            Serialize(T const &object)
            {
                Common::Buffer buffer;

                {
                    boost::iostreams::filtering_ostream stream{boost::iostreams::back_inserter(buffer)};
                    Serialize(object, stream);
                    stream.flush();
                }

                return buffer;
            }

            template <typename T, typename TStream>
            inline typename std::enable_if<!Reflection::IsReflectable<T>(), void>::type
            Serialize(T const &object, TStream &stream, std::string const &rootName = {})
            {
                if (!rootName.empty())
                {
                    boost::json::object root;
                    root[rootName] = Detail::ValueToJson(object);
                    Detail::Write(stream, root);
                }
                else
                {
                    Detail::Write(stream, Detail::ValueToJson(object));
                }
            }

            template <typename T>
            inline typename std::enable_if<!Reflection::IsReflectable<T>(), Common::Buffer>::type
            Serialize(T const &object, std::string const &rootName = {})
            {
                Common::Buffer buffer;

                {
                    boost::iostreams::filtering_ostream stream{boost::iostreams::back_inserter(buffer)};
                    Serialize(object, stream, rootName);
                    stream.flush();
                }

                return buffer;
            }

            template <typename T, typename TStream>
            inline typename std::enable_if<Reflection::IsReflectable<T>(), T>::type
            Deserialize(TStream &stream)
            {
                Common::Buffer buffer;
                std::copy(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>(),
                        std::back_inserter(buffer));
                auto root = boost::json::parse({buffer.data(), buffer.size()});

                T object;
                using BasesType = typename Reflection::Reflect<T>::Base;
                Detail::BasesDeserializer<BasesType, std::tuple_size<BasesType>::value>::Deserialize(root, object);
                Detail::Deserializer<Reflection::Reflect<T>::Fields::Count>::Deserialize(root, object);
                return object;
            }

            template <typename T>
            inline typename std::enable_if<Reflection::IsReflectable<T>(), T>::type
            Deserialize(Common::Buffer const &buffer)
            {
                using SourceType = boost::iostreams::basic_array_source<char>;
                SourceType source{!buffer.empty() ? buffer.data() : nullptr, buffer.size()};
                boost::iostreams::stream<SourceType> stream{source};

                return Deserialize<T>(stream);
            }

            template <typename T, typename TStream>
            inline typename std::enable_if<!Reflection::IsReflectable<T>(), T>::type
            Deserialize(TStream &stream, std::string const &rootName = {})
            {
                Common::Buffer buffer;
                std::copy(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>(),
                        std::back_inserter(buffer));
                auto root = boost::json::parse({buffer.data(), buffer.size()});

                T object{};
                Detail::JsonToValue<T>(rootName.empty() ? root : root.as_object()[rootName], object);
                return object;
            }

            template <typename T>
            inline typename std::enable_if<!Reflection::IsReflectable<T>(), T>::type
            Deserialize(Common::Buffer const &buffer, std::string const &rootName = {})
            {
                using SourceType = boost::iostreams::basic_array_source<char>;
                SourceType source{!buffer.empty() ? buffer.data() : nullptr, buffer.size()};
                boost::iostreams::stream<SourceType> stream{source};

                return Deserialize<T>(stream, rootName);
            }

        }   // namespace Json
    }   // namespace Serialization
}   // namespace Mif

#endif  // !__MIF_SERIALIZATION_JSON_H__
