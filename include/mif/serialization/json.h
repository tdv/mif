//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERIALIZATION_JSON_H__
#define __MIF_SERIALIZATION_JSON_H__

// STD
#include <array>
#include <stdexcept>
#include <cstdint>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

// JSONCPP
#include <json/json.h>
#include <json/reader.h>

// MIF
#include "mif/common/index_sequence.h"
#include "mif/common/static_string.h"
#include "mif/common/unused.h"
#include "mif/reflection/reflection.h"
#include "mif/serialization/traits.h"

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

                    MIF_DECLARE_SRTING_PROVIDER(Id, "id")
                    MIF_DECLARE_SRTING_PROVIDER(Value, "val")

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
                typename std::enable_if<std::is_enum<T>::value, ::Json::Value>::type
                inline ValueToJson(T const &object);

                template <typename T>
                typename std::enable_if<Traits::IsSimple<T>(), ::Json::Value>::type
                ValueToJson(T const &object);

                template <typename T>
                typename std::enable_if<Reflection::IsReflectable<T>(), ::Json::Value>::type
                ValueToJson(T const &object);

                template <typename T>
                typename std::enable_if<Traits::IsSmartPointer<T>(), ::Json::Value>::type
                ValueToJson(T const &ptr);

                template <typename T>
                typename std::enable_if<Traits::IsIterable<T>(), ::Json::Value>::type
                ValueToJson(T const &array);

                template <typename ... T>
                ::Json::Value ValueToJson(std::tuple<T ... > const &tuple);

                template <typename TFirst, typename TSecond>
                ::Json::Value ValueToJson(std::pair<TFirst, TSecond> const &pair);

                template <typename T>
                typename std::enable_if<Traits::IsSimple<T>(), T>::type&
                JsonToValue(::Json::Value const &root, T &object);

                template <typename T>
                inline typename std::enable_if<std::is_enum<T>::value, T>::type&
                JsonToValue(::Json::Value const &root, T &object);

                template <typename T>
                typename std::enable_if<Reflection::IsReflectable<T>(), T>::type&
                JsonToValue(::Json::Value const &root, T &object);

                template <typename T>
                typename std::enable_if<Traits::IsSmartPointer<T>(), T>::type&
                JsonToValue(::Json::Value const &root, T &object);

                template <typename TFirst, typename TSecond>
                std::pair<TFirst, TSecond>&
                JsonToValue(::Json::Value const &root, std::pair<TFirst, TSecond> &pair);

                template <typename ... T>
                std::tuple<T ... >&
                JsonToValue(::Json::Value const &root, std::tuple<T ... > &tuple);

                template <typename T, std::size_t N>
                std::array<T, N>&
                JsonToValue(::Json::Value const &root, std::array<T, N> &array);

                template <typename T>
                inline typename std::enable_if<Traits::IsIterable<T>(), T>::type&
                JsonToValue(::Json::Value const &root, T &object);

                template <typename T>
                typename std::enable_if<std::is_pointer<T>::value, ::Json::Value>::type
                inline ValueToJson(T const &)
                {
                    static_assert(!std::is_pointer<T>::value, "[Mif::Serialization::Json::Detail] You can't serialize the raw pointers.");
                }

                template <typename T>
                typename std::enable_if<std::is_enum<T>::value, ::Json::Value>::type
                inline ValueToJson(T const &object)
                {
                    return ValueToJson(static_cast<typename std::underlying_type<T>::type>(object));
                }

                template <typename T>
                typename std::enable_if<Traits::IsSimple<T>(), ::Json::Value>::type
                inline ValueToJson(T const &object)
                {
                    return {object};
                }

                template <typename T>
                typename std::enable_if<Reflection::IsReflectable<T>(), ::Json::Value>::type
                inline ValueToJson(T const &object)
                {
                    ::Json::Value root{::Json::objectValue};
                    using BasesType = typename Reflection::Reflect<T>::Base;
                    BasesSerializer<BasesType, std::tuple_size<BasesType>::value>::Serialize(root, object);
                    Serializer<Reflection::Reflect<T>::Fields::Count>::Serialize(root, object);
                    return root;
                }

                template <typename T>
                typename std::enable_if<Traits::IsSmartPointer<T>(), ::Json::Value>::type
                inline ValueToJson(T const &ptr)
                {
                    if (!ptr)
                        return {::Json::nullValue};
                    return ValueToJson(*ptr);
                }

                template <typename T>
                typename std::enable_if<Traits::IsIterable<T>(), ::Json::Value>::type
                inline ValueToJson(T const &array)
                {
                    ::Json::Value root{::Json::arrayValue};

                    for (auto const &i : array)
                        root.append(ValueToJson(i));

                    return root;
                }

                template <typename T, std::size_t ... Indexes>
                inline ::Json::Value TupleToJson(T const &tuple, Common::IndexSequence<Indexes ... > const *)
                {
                    ::Json::Value root(::Json::arrayValue);

                    Common::Unused(root[static_cast<::Json::Value::ArrayIndex>(Indexes)] = ValueToJson(std::get<Indexes>(tuple)) ... );

                    return root;
                }

                template <typename ... T>
                inline ::Json::Value ValueToJson(std::tuple<T ... > const &tuple)
                {
                    using TupleType = std::tuple<T ... >;
                    return TupleToJson(tuple, static_cast<Common::MakeIndexSequence<std::tuple_size<TupleType>::value> const *>(nullptr));
                }

                template <typename TFirst, typename TSecond>
                inline ::Json::Value ValueToJson(std::pair<TFirst, TSecond> const &pair)
                {
                    ::Json::Value root{::Json::objectValue};

                    root[Tag::Id::GetString()] = ValueToJson(pair.first);
                    root[Tag::Value::GetString()] = ValueToJson(pair.second);

                    return root;
                }

                template <typename TBases, std::size_t I>
                struct BasesSerializer
                {
                    template <typename T>
                    static void Serialize(::Json::Value &root, T const &object)
                    {
                        BasesSerializer<TBases, I - 1>::Serialize(root, object);
                        using BaseType = typename std::tuple_element<I - 1, TBases>::type;
                        root[Reflection::Reflect<BaseType>::Name::GetString()] = ValueToJson(static_cast<BaseType const &>(object));
                    }
                };

                template <typename TBases>
                struct BasesSerializer<TBases, 0>
                {
                    template <typename T>
                    static void Serialize(::Json::Value &, T const &)
                    {
                    }
                };

                template <std::size_t I>
                struct Serializer
                {
                    template <typename T>
                    static void Serialize(::Json::Value &root, T const &object)
                    {
                        Serializer<I - 1>::Serialize(root, object);
                        using FieldType = typename Reflection::Reflect<T>::Fields::template Field<I - 1>;
                        root[FieldType::Name::GetString()] = ValueToJson(object.*FieldType::Access());
                    }
                };

                template <>
                struct Serializer<0>
                {
                    template <typename T>
                    static void Serialize(::Json::Value &, T &)
                    {
                    }
                };

                template <typename T>
                struct JsonValueConverter;

                template <>
                struct JsonValueConverter<std::string>
                {
                    static std::string Convert(::Json::Value const &val)
                    {
                        if (!val.isString() && !val.isConvertibleTo(::Json::stringValue))
                            throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to string."};
                        return val.asString();
                    }
                };

                template <>
                struct JsonValueConverter<double>
                {
                    static double Convert(::Json::Value const &val)
                    {
                        if (!val.isDouble() && !val.isConvertibleTo(::Json::realValue))
                            throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to double."};
                        return val.asDouble();
                    }
                };

                template <>
                struct JsonValueConverter<bool>
                {
                    static bool Convert(::Json::Value const &val)
                    {
                        if (!val.isBool() && !val.isConvertibleTo(::Json::booleanValue))
                            throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to bool."};
                        return val.asBool();
                    }
                };

                template <>
                struct JsonValueConverter<std::int64_t>
                {
                    static std::int64_t Convert(::Json::Value const &val)
                    {
                        if (!val.isInt64() && !val.isConvertibleTo(::Json::intValue))
                            throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to int64."};
                        return val.asInt64();
                    }
                };

                template <>
                struct JsonValueConverter<std::uint64_t>
                {
                    static std::uint64_t Convert(::Json::Value const &val)
                    {
                        if (!val.isUInt64() && !val.isConvertibleTo(::Json::intValue))
                            throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to uint64."};
                        return val.asUInt64();
                    }
                };

                template <>
                struct JsonValueConverter<std::int32_t>
                {
                    static std::int32_t Convert(::Json::Value const &val)
                    {
                        if (!val.isInt() && !val.isConvertibleTo(::Json::intValue))
                            throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to int32."};
                        return val.asInt();
                    }
                };

                template <>
                struct JsonValueConverter<std::uint32_t>
                {
                    static std::uint32_t Convert(::Json::Value const &val)
                    {
                        if (!val.isUInt() && !val.isConvertibleTo(::Json::intValue))
                            throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonValueConverter::Convert] Failed to convert to uint32."};
                        return val.asUInt();
                    }
                };

                template <>
                struct JsonValueConverter<std::int16_t>
                {
                    static std::int16_t Convert(::Json::Value const &val)
                    {
                        return static_cast<std::int16_t>(JsonValueConverter<std::int32_t>::Convert(val));
                    }
                };

                template <>
                struct JsonValueConverter<std::uint16_t>
                {
                    static std::uint16_t Convert(::Json::Value const &val)
                    {
                        return static_cast<std::uint16_t>(JsonValueConverter<std::int32_t>::Convert(val));
                    }
                };

                template <>
                struct JsonValueConverter<std::int8_t>
                {
                    static std::int8_t Convert(::Json::Value const &val)
                    {
                        return static_cast<std::int8_t>(JsonValueConverter<std::int32_t>::Convert(val));
                    }
                };

                template <>
                struct JsonValueConverter<std::uint8_t>
                {
                    static std::uint8_t Convert(::Json::Value const &val)
                    {
                        return static_cast<std::uint8_t>(JsonValueConverter<std::int32_t>::Convert(val));
                    }
                };

                template <typename T>
                inline typename std::enable_if<Traits::IsSimple<T>(), T>::type&
                JsonToValue(::Json::Value const &root, T &object)
                {
                    if (root.isNull())
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to get value from null."};
                    object = JsonValueConverter<T>::Convert(root);
                    return object;
                }

                template <typename T>
                inline typename std::enable_if<std::is_enum<T>::value, T>::type&
                JsonToValue(::Json::Value const &root, T &object)
                {
                    if (root.isNull())
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to get value from null."};
                    object = static_cast<T>(JsonValueConverter<typename std::underlying_type<T>::type>::Convert(root));
                    return object;
                }

                template <typename T>
                inline typename std::enable_if<Reflection::IsReflectable<T>(), T>::type&
                JsonToValue(::Json::Value const &root, T &object)
                {
                    if (root.isNull() || (root.type() != ::Json::objectValue && !root.isConvertibleTo(::Json::objectValue)))
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to get value. "
                            "Json element is invalid or is not converted to the correct type."};
                    }

                    using BasesType = typename Reflection::Reflect<T>::Base;
                    BasesDeserializer<BasesType, std::tuple_size<BasesType>::value>::Deserialize(root, object);

                    Deserializer<Reflection::Reflect<T>::Fields::Count>::Deserialize(root, object);
                    return object;
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsSmartPointer<T>(), T>::type&
                JsonToValue(::Json::Value const &root, T &object)
                {
                    if (root.isNull())
                        return object;

                    using ObjectType = typename T::element_type;
                    object.reset(new ObjectType{});
                    JsonToValue<ObjectType>(root, *object);

                    return object;
                }

                template <typename TFirst, typename TSecond>
                inline std::pair<TFirst, TSecond>&
                JsonToValue(::Json::Value const &root, std::pair<TFirst, TSecond> &pair)
                {
                    if ((root.isNull() || !root.isMember(Tag::Id::GetString()) || !root.isMember(Tag::Value::GetString())) ||
                        (root.type() != ::Json::objectValue && !root.isConvertibleTo(::Json::objectValue)))
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to parse pair. "
                            "Value is null or json has no pair type object."};
                    }

                    JsonToValue(root.get(Tag::Id::GetString(), ::Json::Value{}),
                        const_cast<typename std::remove_const<TFirst>::type &>(pair.first));
                    JsonToValue(root.get(Tag::Value::GetString(), ::Json::Value{}), pair.second);

                    return pair;
                }

                template <typename T, std::size_t ... Indexes>
                inline void JsonToTuple(::Json::Value const &root, T &tuple, Common::IndexSequence<Indexes ... > const *)
                {
                    Common::Unused(JsonToValue(root.get(static_cast<::Json::Value::ArrayIndex>(Indexes), ::Json::Value{}), std::get<Indexes>(tuple)) ... );
                }

                template <typename ... T>
                inline std::tuple<T ... >&
                JsonToValue(::Json::Value const &root, std::tuple<T ... > &tuple)
                {
                    if (root.isNull() || (root.type() != ::Json::arrayValue && !root.isConvertibleTo(::Json::arrayValue)))
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to parse pair. "
                            "Value is null or json has no tuple type object."};
                    }

                    using TupleType = std::tuple<T ... >;
                    JsonToTuple(root, tuple, static_cast<Common::MakeIndexSequence<std::tuple_size<TupleType>::value> const *>(nullptr));

                    return tuple;
                }

                template <typename T, std::size_t ... Indexes>
                inline void JsonToArray(::Json::Value const &root, std::array<T, sizeof ... (Indexes)> &array, Common::IndexSequence<Indexes ... > const *)
                {
                    Common::Unused(JsonToValue(root.get(static_cast<::Json::Value::ArrayIndex>(Indexes), ::Json::Value{}), array[Indexes]) ... );
                }

                template <typename T, std::size_t N>
                inline std::array<T, N>&
                JsonToValue(::Json::Value const &root, std::array<T, N> &array)
                {
                    if (root.isNull() || (root.type() != ::Json::arrayValue && !root.isConvertibleTo(::Json::arrayValue)))
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to parse pair. "
                            "Value is null or json has no array type object."};
                    }

                    std::array<T, N> tmp;
                    JsonToArray(root, tmp, static_cast<Common::MakeIndexSequence<N> const *>(nullptr));
                    std::swap(array, tmp);

                    return array;
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsIterable<T>(), T>::type&
                JsonToValue(::Json::Value const &root, T &object)
                {
                    if (root.isNull() || (root.type() != ::Json::arrayValue && !root.isConvertibleTo(::Json::arrayValue)))
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Json::Detail::JsonToValue] Failed to get value. "
                            "Json element is invalid or is not converted to the array."};
                    }

                    T{}.swap(object);

                    if (!root.size())
                        return object;

                    using ObjectType = typename T::value_type;

                    for (auto const &i : root)
                    {
                        ObjectType item;
                        JsonToValue(i, item);
                        *std::inserter(object, std::end(object)) = std::move(item);
                    }

                    return object;
                }

                template <typename TBases, std::size_t I>
                struct BasesDeserializer
                {
                    template <typename T>
                    static void Deserialize(::Json::Value const &root, T &object)
                    {
                        BasesDeserializer<TBases, I - 1>::Deserialize(root, object);
                        using BaseType = typename std::tuple_element<I - 1, TBases>::type;
                        JsonToValue(root.get(Reflection::Reflect<BaseType>::Name::GetString(), ::Json::Value{}), static_cast<BaseType &>(object));
                    }
                };

                template <typename TBases>
                struct BasesDeserializer<TBases, 0>
                {
                    template <typename T>
                    static void Deserialize(::Json::Value const &, T &)
                    {
                    }
                };


                template <std::size_t I>
                struct Deserializer
                {
                    template <typename T>
                    static void Deserialize(::Json::Value const &root, T &object)
                    {
                        Deserializer<I - 1>::Deserialize(root, object);
                        using FieldType = typename Reflection::Reflect<T>::Fields::template Field<I - 1>;
                        JsonToValue(root.get(FieldType::Name::GetString(), ::Json::Value{}), object.*FieldType::Access());
                    }
                };

                template <>
                struct Deserializer<0>
                {
                    template <typename T>
                    static void Deserialize(::Json::Value const &, T &)
                    {
                    }
                };

            }   // namespace Detail

            template <typename T, typename TStream>
            inline void Serialize(T const &object, TStream &stream)
            {
                ::Json::Value root{::Json::objectValue};
                using BasesType = typename Reflection::Reflect<T>::Base;
                Detail::BasesSerializer<BasesType, std::tuple_size<BasesType>::value>::Serialize(root, object);
                Detail::Serializer<Reflection::Reflect<T>::Fields::Count>::Serialize(root, object);
                stream << root;
            }

            template <typename T, typename TStream>
            inline T Deserialize(TStream &stream)
            {
                ::Json::Value root;
                ::Json::Reader reader;
                if (!reader.parse(stream, root))
                {
                    throw std::invalid_argument{"[Mif::Serialization::Json::Deserialize] Failed to parse json. Error: " +
                        reader.getFormattedErrorMessages()};
                }
                T object;
                using BasesType = typename Reflection::Reflect<T>::Base;
                Detail::BasesDeserializer<BasesType, std::tuple_size<BasesType>::value>::Deserialize(root, object);
                Detail::Deserializer<Reflection::Reflect<T>::Fields::Count>::Deserialize(root, object);
                return object;
            }

        }   // namespace Json
    }   // namespace Serialization
}   // namespace Mif

#endif  // !__MIF_SERIALIZATION_JSON_H__
