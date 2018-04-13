//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2018 tdv
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

// PUGIXML
#include <pugixml.hpp>

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

                using NodeType = pugi::xml_node;

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
                    pugi::xml_document doc;

                    auto decl = doc.append_child(pugi::xml_node_type::node_declaration);
                    decl.append_attribute("version") = "1.0";
                    decl.append_attribute("encoding") = "UTF-8";

                    Serialize(doc, object, root);

                    doc.save(stream);
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
                    auto item = node.append_child(itemName.c_str());
                    SerializeBase<Bases, 0>(item, object);
                    Serialize<0, Meta::Fields::Count>(item, object);
                }

                template <typename T>
                inline typename std::enable_if<Reflection::IsReflectable<T>() && std::is_enum<T>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name)
                {
                    node.append_child(name.c_str())
                            .append_child(pugi::xml_node_type::node_pcdata)
                            .set_value(Reflection::ToString(object).c_str()  );
                }

                template <typename T>
                inline typename std::enable_if<!Reflection::IsReflectable<T>() && std::is_enum<T>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name)
                {
                    using Type = typename std::underlying_type<T>::type;
                    node.append_child(name.c_str())
                            .append_child(pugi::xml_node_type::node_pcdata)
                            .set_value(std::to_string(static_cast<Type>(object)).c_str());
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsIterable<T>(), void>::type
                Serialize(NodeType &node, T const &object, std::string const &name)
                {
                    auto item = node.append_child(name.c_str());
                    for (auto const &i : object)
                        Serialize(item, i, Tag::Item::Value);
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsSimple<T>() && !std::is_enum<T>::value && !std::is_same<T, std::string>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name)
                {
                    node.append_child(name.c_str())
                            .append_child(pugi::xml_node_type::node_pcdata)
                            .set_value(std::to_string(object).c_str());
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsSimple<T>() && std::is_same<T, std::string>::value, void>::type
                Serialize(NodeType &node, T const &object, std::string const &name)
                {
                    node.append_child(name.c_str())
                            .append_child(pugi::xml_node_type::node_cdata)
                            .set_value(object.c_str());
                }

                template <typename TFirst, typename TSecond>
                inline void Serialize(NodeType &node, std::pair<TFirst, TSecond> const &object, std::string const &name)
                {
                    auto item = node.append_child(name.c_str());
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
                        node.append_child(name.c_str());
                }

                template <typename ... T>
                inline void Serialize(NodeType &node, std::tuple<T ... > const &object, std::string const &name)
                {
                    auto item = node.append_child(name.c_str());
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
                    pugi::xml_document doc;

                    auto result = doc.load(stream);
                    if (!result)
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Xml::Deserialize] Failed to parse xml. Error: " +
                            std::string{result.description()}};
                    }

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
                    auto const item = node.child((!name.empty() ? name : Meta::Name::Value).c_str());
                    DeserializeBase<Bases, 0>(item, object);
                    Deserialize<0, Meta::Fields::Count>(item, object);
                }

                template <typename T>
                inline typename std::enable_if<Reflection::IsReflectable<T>() && std::is_enum<T>::value, void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name)
                {
                    if (node)
                    {
                        if (auto const item = node.child(name.c_str()))
                        {

                            if (auto const *value = item.child_value())
                            {
                                object = Reflection::FromString<T>(value);
                            }
                            else
                            {
                                throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                        "Failed to parse enum value. No value in the node \"" + name + "\"."};
                            }

                        }
                        else
                        {
                            throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                    "Failed to parse enum value. No node \"" + name + "\"."};
                        }
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
                    if (node)
                    {
                        if (auto const item = node.child(name.c_str()))
                        {

                            if (auto const *value = item.child_value())
                            {
                                object = static_cast<T>(std::stoull(value));
                            }
                            else
                            {
                                throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                        "Failed to parse enum value. No value in the node \"" + name + "\"."};
                            }

                        }
                        else
                        {
                            throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                    "Failed to parse enum value. No node \"" + name + "\"."};
                        }
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
                    if (auto const items = node.child(name.c_str()))
                    {
                        for (auto i = items.child(Tag::Item::Value); i ; i = i.next_sibling())
                        {
                            using Type = typename T::value_type;
                            Type data;
                            pugi::xml_document tmp;
                            tmp.append_copy(i);
                            Deserialize(tmp, data, Tag::Item::Value);
                            *std::inserter(object, std::end(object)) = std::move(data);
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
                    if (auto const item = node.child(name.c_str()))
                    {
                        std::stringstream{item.child_value()} >> object;
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
                    if (auto const item = node.child(name.c_str()))
                    {
                        object = item.child_value();
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
                    if (node)
                    {
                        Deserialize(node, const_cast<typename std::remove_const<TFirst>::type &>(object.first), Tag::Id::Value);
                        Deserialize(node, object.second, Tag::Value::Value);
                    }
                    else
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                "Failed to parse pair. No node \"" + name + "\"."};
                    }
                }

                template <typename T>
                inline typename std::enable_if<Traits::IsSmartPointer<T>(), void>::type
                Deserialize(NodeType const &node, T &object, std::string const &name)
                {
                    if (node)
                    {
                        using Type = typename T::element_type;
                        object.reset(new Type{});
                        Deserialize(node, *object, name);
                    }
                    else
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                "Failed to parse item. No node \"" + name + "\"."};
                    }
                }

                template <std::size_t I, std::size_t N, typename T>
                inline typename std::enable_if<I != N, void>::type
                Deserialize(NodeType &node, T &object)
                {
                    if (!node)
                    {
                        throw std::invalid_argument{"[Mif::Serialization::Xml::Detail::Deserialize] "
                                "Failed to parse tuple item."};
                    }

                    Deserialize(node, std::get<I>(object), Tag::Item::Value);

                    node.remove_child(Tag::Item::Value);

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
                    if (auto item = node.child(name.c_str()))
                    {
                        Deserialize<0, sizeof ... (T)>(item, object);
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
                    if (auto item = node.child(name.c_str()))
                    {
                        Deserialize<0, N>(item, object);
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
