//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERIALIZATION_XML_H__
#define __MIF_SERIALIZATION_XML_H__

// STD
#include <array>
#include <stdexcept>
#include <cstdint>
#include <iterator>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

// BOOST
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


// MIF
#include "mif/common/types.h"
#include "mif/common/static_string.h"
#include "mif/common/unused.h"
#include "mif/reflection/reflection.h"
#include "mif/serialization/traits.h"

namespace Mif
{
    namespace Serialization
    {
        namespace Xml
        {
            namespace Detail
            {
                namespace Tag
                {

                    using Item = MIF_STATIC_STR("item");
                    using Id = MIF_STATIC_STR("id");
                    using Value = MIF_STATIC_STR("val");

                }   // namespace Tag

                using NodeType = boost::property_tree::ptree;

                //--------------------------------------------------------------------------------------------------------------------------

                template <typename T>
                void Serialize(std::ostream &stream, T const &object, std::string const &root = {});

                template <typename TBases, std::size_t I, typename T>
                typename std::enable_if<I != std::tuple_size<TBases>::value, void>::type
                SerializeBase(NodeType &node, T const &object);

                template <typename TBases, std::size_t I, typename T>
                typename std::enable_if<I == std::tuple_size<TBases>::value, void>::type
                SerializeBase(NodeType &node, T const &object);

                template <typename T>
                typename std::enable_if<Reflection::IsReflectable<T>() && !std::is_enum<T>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name);

                template <typename T>
                typename std::enable_if<Reflection::IsReflectable<T>() && std::is_enum<T>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name);

                template <typename T>
                typename std::enable_if<!Reflection::IsReflectable<T>() && std::is_enum<T>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name);

                template <typename T>
                typename std::enable_if<Traits::IsIterable<T>(), void>::type
                Serialize(NodeType &node, T const &object, std::string const &name);

                template <typename T>
                typename std::enable_if<Traits::IsSimple<T>() && !std::is_enum<T>::value && !std::is_same<T, std::string>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name);

                template <typename T>
                typename std::enable_if<Traits::IsSimple<T>() && std::is_same<T, std::string>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name);

                template <typename TFirst, typename TSecond>
                void Serialize(NodeType &node, std::pair<TFirst, TSecond> const &object, std::string const &name);

                template <typename T>
                typename std::enable_if<Traits::IsSmartPointer<T>(), void>::type
                Serialize(NodeType &node, T const &object, std::string const &name);

                template <typename ... T>
                void Serialize(NodeType &node, std::tuple<T ... > const &object, std::string const &name);

                template <std::size_t I, std::size_t N, typename ... T>
                typename std::enable_if<I != N, void>::type
                Serialize(NodeType &node, std::tuple<T ... > const &object);

                template <std::size_t I, std::size_t N, typename ... T>
                typename std::enable_if<I == N, void>::type
                Serialize(NodeType &node, std::tuple<T ... > const &object);

                template <std::size_t I, std::size_t N, typename T>
                typename std::enable_if<I != N, void>::type
                Serialize(NodeType &node, T const &object);

                template <std::size_t I, std::size_t N, typename T>
                typename std::enable_if<I == N, void>::type
                Serialize(NodeType &node, T const &object);

                //--------------------------------------------------------------------------------------------------------------------------

                template <typename T>
                void Deserialize(std::istream &stream, T &object, std::string const &root = {});

                template <typename TBases, std::size_t I, typename T>
                typename std::enable_if<I != std::tuple_size<TBases>::value, void>::type
                DeserializeBase(NodeType const &node, T &object);

                template <typename TBases, std::size_t I, typename T>
                typename std::enable_if<I == std::tuple_size<TBases>::value, void>::type
                DeserializeBase(NodeType const &node, T &object);

                template <typename T>
                typename std::enable_if<Reflection::IsReflectable<T>() && !std::is_enum<T>::value, void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name);

                template <typename T>
                typename std::enable_if<Reflection::IsReflectable<T>() && std::is_enum<T>::value, void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name);

                template <typename T>
                typename std::enable_if<!Reflection::IsReflectable<T>() && std::is_enum<T>::value, void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name);

                template <std::size_t I, std::size_t N, typename T>
                typename std::enable_if<I != N, void>::type
                Deserialize(NodeType const &node, T &object);

                template <std::size_t I, std::size_t N, typename T>
                typename std::enable_if<I == N, void>::type
                Deserialize(NodeType const &node, T &object);

                template <typename T>
                typename std::enable_if<Traits::IsIterable<T>(), void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name);

                template <typename T>
                typename std::enable_if<Traits::IsSimple<T>() && !std::is_enum<T>::value && !std::is_same<T, std::string>::value, void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name);

                template <typename T>
                typename std::enable_if<Traits::IsSimple<T>() && std::is_same<T, std::string>::value, void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name);

                template <typename TFirst, typename TSecond>
                void Deserialize(NodeType const &node, std::pair<TFirst, TSecond> &object, std::string const &name);

                template <typename T>
                typename std::enable_if<Traits::IsSmartPointer<T>(), void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name);

                template <std::size_t I, std::size_t N, typename T>
                typename std::enable_if<I != N, void>::type
                Deserialize(NodeType &node, T &object);

                template <std::size_t I, std::size_t N, typename T>
                typename std::enable_if<I == N, void>::type
                Deserialize(NodeType &node, T &object);

                template <typename ... T>
                void Deserialize(NodeType const &node, std::tuple<T ... > &object, std::string const &name);

                template <typename T, std::size_t N>
                void Deserialize(NodeType const &node, std::array<T, N> &object, std::string const &name);

                //--------------------------------------------------------------------------------------------------------------------------

                template <typename T>
                inline void Serialize(std::ostream &stream, T const &object, std::string const &root)
                {
                    NodeType doc;

                    Serialize(doc, object, root);

                    auto const settings = boost::property_tree::xml_writer_make_settings<std::string>(
                            '\t', 1, "utf-8");

                    boost::property_tree::write_xml(stream, doc, settings);
                }


                template <typename TBases, std::size_t I, typename T>
                inline typename std::enable_if<I != std::tuple_size<TBases>::value, void>::type
                SerializeBase(NodeType &node, T const &object)
                {
                    using Base = typename std::tuple_element<I, TBases>::type;
                    Serialize(node, static_cast<Base const &>(object),
                            Reflection::Reflect<Base>::FullName::Value);
                    SerializeBase<TBases, I + 1>(node, object);
                }

                template <typename TBases, std::size_t I, typename T>
                inline typename std::enable_if<I == std::tuple_size<TBases>::value, void>::type
                SerializeBase(NodeType &node, T const &object)
                {
                    Common::Unused(node);
                    Common::Unused(object);
                }

                template <typename T>
                inline typename std::enable_if<Reflection::IsReflectable<T>() && !std::is_enum<T>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name)
                {
                    using Meta = Reflection::Reflect<T>;
                    using Bases = typename Meta::Base;
                    auto const itemName = !name.empty() ? name : Meta::Name::Value;
                    auto &item = node.add(itemName, "");
                    SerializeBase<Bases, 0>(item, object);
                    Serialize<0, Meta::Fields::Count>(item, object);
                }

                template <typename T>
                inline typename std::enable_if<Reflection::IsReflectable<T>() && std::is_enum<T>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name)
                {
                    node.add(name, Reflection::ToString(object));
                }

                template <typename T>
                inline typename std::enable_if<!Reflection::IsReflectable<T>() && std::is_enum<T>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name)
                {
                    using Type = typename std::underlying_type<T>::type;
                    node.add(name, std::to_string(static_cast<Type>(object)));
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsIterable<T>(), void>::type
                Serialize(NodeType &node, T const &object, std::string const &name)
                {
                    auto &item = node.add(name, "");
                    for (auto const &i : object)
                        Serialize(item, i, Tag::Item::Value);
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsSimple<T>() && !std::is_enum<T>::value && !std::is_same<T, std::string>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name)
                {
                    node.add(name, std::to_string(object));
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsSimple<T>() && std::is_same<T, std::string>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name)
                {
                    node.add(name, object);
                }

                template <typename TFirst, typename TSecond>
                inline void Serialize(NodeType &node, std::pair<TFirst, TSecond> const &object, std::string const &name)
                {
                    auto &item = node.add(name, "");
                    Serialize(item, object.first, Tag::Id::Value);
                    Serialize(item, object.second, Tag::Value::Value);
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsSmartPointer<T>(), void>::type
                Serialize(NodeType &node, T const &object, std::string const &name)
                {
                    if (object)
                        Serialize(node, *object, name);
                    else
                        node.add(name, "");
                }

                template <typename ... T>
                inline void Serialize(NodeType &node, std::tuple<T ... > const &object, std::string const &name)
                {
                    auto item = node.add(name, "");
                    Serialize<0, sizeof ... (T)>(item, object);
                }

                template <std::size_t I, std::size_t N, typename ... T>
                inline typename std::enable_if<I != N, void>::type
                Serialize(NodeType &node, std::tuple<T ... > const &object)
                {
                    Serialize(node, std::get<I>(object), Tag::Item::Value);
                    Serialize<I + 1, N>(node, object);
                }

                template <std::size_t I, std::size_t N, typename ... T>
                inline typename std::enable_if<I == N, void>::type
                Serialize(NodeType &node, std::tuple<T ... > const &object)
                {
                    Common::Unused(node, object);
                }

                template <std::size_t I, std::size_t N, typename T>
                inline typename std::enable_if<I != N, void>::type
                Serialize(NodeType &node, T const &object)
                {
                    using Field = typename Reflection::Reflect<T>::Fields::template Field<I>;
                    Serialize(node, object.*Field::Access(), Field::Name::Value);
                    Serialize<I + 1, N>(node, object);
                }

                template <std::size_t I, std::size_t N, typename T>
                inline typename std::enable_if<I == N, void>::type
                Serialize(NodeType &node, T const &object)
                {
                    Common::Unused(node, object);
                }

                //--------------------------------------------------------------------------------------------------------------------------

                template <typename T>
                inline void Deserialize(std::istream &stream, T &object, std::string const &root)
                {
                    {
                        T tmp;
                        std::swap(tmp, object);
                    }

                    NodeType doc;

                    boost::property_tree::ptree tree;
                    boost::property_tree::xml_parser::read_xml(stream, doc);

                    Deserialize(doc, object, root);
                }

                template <typename TBases, std::size_t I, typename T>
                inline typename std::enable_if<I != std::tuple_size<TBases>::value, void>::type
                DeserializeBase(NodeType const &node, T &object)
                {
                    using Base = typename std::tuple_element<I, TBases>::type;
                    Deserialize(node, static_cast<Base &>(object),
                            Reflection::Reflect<Base>::FullName::Value);
                    DeserializeBase<TBases, I + 1>(node, object);
                }

                template <typename TBases, std::size_t I, typename T>
                inline typename std::enable_if<I == std::tuple_size<TBases>::value, void>::type
                DeserializeBase(NodeType const &node, T &object)
                {
                    Common::Unused(node);
                    Common::Unused(object);
                }

                template <typename T>
                inline typename std::enable_if<Reflection::IsReflectable<T>() && !std::is_enum<T>::value, void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name)
                {
                    using Meta = Reflection::Reflect<T>;
                    using Bases = typename Meta::Base;
                    if (auto const item = node.get_child_optional(Meta::Name::Value))
                    {
                        DeserializeBase<Bases, 0>(item.get(), object);
                        Deserialize<0, Meta::Fields::Count>(item.get(), object);
                    }
                    else
                    {
                        auto const itemObj = node.get_child(name);
                        DeserializeBase<Bases, 0>(itemObj, object);
                        Deserialize<0, Meta::Fields::Count>(itemObj, object);
                    }
                }

                template <typename T>
                inline typename std::enable_if<Reflection::IsReflectable<T>() && std::is_enum<T>::value, void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name)
                {
                    if (auto const value = node.get_optional<std::string>(name))
                    {
                            object = Reflection::FromString<T>(value.get());
                        }
                        else
                        {
                            throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                    "Failed to parse enum value. No node \"" + name + "\"."};
                        }
                }

                template <typename T>
                inline typename std::enable_if<!Reflection::IsReflectable<T>() && std::is_enum<T>::value, void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name)
                {
                    if (auto const value = node.get_optional<T>(name))
                    {
                        object = value.get();
                    }
                    else
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                "Failed to parse enum value. No node \"" + name + "\"."};
                    }
                }

                template <std::size_t I, std::size_t N, typename T>
                inline typename std::enable_if<I != N, void>::type
                Deserialize(NodeType const &node, T &object)
                {
                    using Meta = typename Reflection::Reflect<T>;
                    using Field = typename Meta::Fields::template Field<I>;
                    Deserialize(node, object.*Field::Access(), Field::Name::Value);
                    Deserialize<I + 1, N>(node, object);
                }

                template <std::size_t I, std::size_t N, typename T>
                inline typename std::enable_if<I == N, void>::type
                Deserialize(NodeType const &node, T &object)
                {
                    Common::Unused(node);
                    Common::Unused(object);
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsIterable<T>(), void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name)
                {
                    {
                        T tmp;
                        std::swap(tmp, object);
                    }
                    if (auto const items = node.get_child_optional(name))
                    {
                        auto inserter = std::inserter(object, std::end(object));
                        for (auto const &i : items.get())
                        {
                            using Type = typename T::value_type;
                            Type data;
                            Deserialize(i.second, data, "");
                            *inserter = std::move(data);
                        }
                    }
                    else
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                "Failed to parse container. No node \"" + name + "\"."};
                    }
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsSimple<T>() && !std::is_enum<T>::value && !std::is_same<T, std::string>::value, void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name)
                {
                    if (auto const item = node.get_optional<std::string>(name))
                    {
                        std::stringstream{item.get()} >> object;
                    }
                    else
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                "Failed to parse value. No node \"" + name + "\"."};
                    }
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsSimple<T>() && std::is_same<T, std::string>::value, void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name)
                {
                    if (auto const item = node.get_optional<std::string>(name))
                    {
                        object = item.get();
                    }
                    else
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                "Failed to parse string. No node \"" + name + "\"."};
                    }
                }

                template <typename TFirst, typename TSecond>
                inline void Deserialize(NodeType const &node, std::pair<TFirst, TSecond> &object, std::string const &name)
                {
                    Deserialize(node, const_cast<typename std::remove_const<TFirst>::type &>(object.first), Tag::Id::Value);
                    Deserialize(node, object.second, Tag::Value::Value);
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsSmartPointer<T>(), void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name)
                {
                    using Type = typename T::element_type;
                    object.reset(new Type{});
                    Deserialize(node, *object, name);
                }

                template <std::size_t I, std::size_t N, typename T>
                inline typename std::enable_if<I != N, void>::type
                Deserialize(NodeType &node, T &object)
                {
                    Deserialize(node, std::get<I>(object), Tag::Item::Value);
                    Deserialize<I + 1, N>(node, object);
                }

                template <std::size_t I, std::size_t N, typename T>
                inline typename std::enable_if<I == N, void>::type
                Deserialize(NodeType &node, T &object)
                {
                    Common::Unused(node, object);
                }

                template <typename ... T>
                inline void Deserialize(NodeType const &node, std::tuple<T ... > &object, std::string const &name)
                {
                    if (auto item = node.get_child_optional(name))
                    {
                        Deserialize<0, sizeof ... (T)>(item.get(), object);
                    }
                    else
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                "Failed to parse tuple. No node \"" + name + "\"."};
                    }
                }

                template <typename T, std::size_t N>
                inline void Deserialize(NodeType const &node, std::array<T, N> &object, std::string const &name)
                {
                    if (auto item = node.get_child_optional(name))
                    {
                        Deserialize<0, N>(item.get(), object);
                    }
                    else
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                "Failed to parse array. No node \"" + name + "\"."};
                    }
                }

            }   // namespace Detail

            template <typename T>
            inline void Serialize(T const &object, std::ostream &stream, std::string const &root = {})
            {
                Detail::Serialize(stream, object, root);
            }

            template <typename T>
            inline Common::Buffer Serialize(T const &object, std::string const &root = {})
            {
                Common::Buffer buffer;

                {
                    boost::iostreams::filtering_ostream stream{boost::iostreams::back_inserter(buffer)};
                    Serialize(object, stream, root);
                    stream.flush();
                }

                return buffer;
            }

            template <typename T>
            inline T Deserialize(std::istream &stream, std::string const &root = {})
            {
                T object;
                Detail::Deserialize(stream, object, root);
                return object;
            }

            template <typename T>
            inline T Deserialize(Common::Buffer const &buffer, std::string const &root = {})
            {
                using SourceType = boost::iostreams::basic_array_source<char>;
                SourceType source{!buffer.empty() ? buffer.data() : nullptr, buffer.size()};
                boost::iostreams::stream<SourceType> stream{source};

                return Deserialize<T>(stream, root);
            }

        }   // namespace Xml
    }   // namespace Serialization
}   // namespace Mif

#endif  // !__MIF_SERIALIZATION_XML_H__
