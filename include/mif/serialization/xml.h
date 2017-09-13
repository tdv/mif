//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERIALIZATION_XML_H__
#define __MIF_SERIALIZATION_XML_H__

// STD
#include <array>
#include <stdexcept>
#include <cstdint>
#include <ostream>
#include <string>
#include <sstream>
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
#include "mif/common/index_sequence.h"
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

                    MIF_DECLARE_SRTING_PROVIDER(Item, "item")
                    MIF_DECLARE_SRTING_PROVIDER(Id, "id")
                    MIF_DECLARE_SRTING_PROVIDER(Value, "val")

                }   // namespace Tag

                using NodeType = pugi::xml_node;

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
                            Reflection::Reflect<Base>::FullName::GetString());
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
                    auto const itemName = !name.empty() ? name : Meta::Name::GetString();
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
                        Serialize(item, i, Tag::Item::GetString());
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
                    Serialize(item, object.first, Tag::Id::GetString());
                    Serialize(item, object.second, Tag::Value::GetString());
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
                    Serialize(node, std::get<I>(object), Tag::Item::GetString());
                    Serialize<I + 1, N>(node, object);
                }

                template <std::size_t I, std::size_t N, typename ... T>
                inline typename std::enable_if<I == N, void>::type
                Serialize(NodeType &node, std::tuple<T ... > const &object)
                {
                    Common::Unused(node);
                    Common::Unused(object);
                }

                template <std::size_t I, std::size_t N, typename T>
                inline typename std::enable_if<I != N, void>::type
                Serialize(NodeType &node, T const &object)
                {
                    using Field = typename Reflection::Reflect<T>::Fields::template Field<I>;
                    Serialize(node, object.*Field::Access(), Field::Name::GetString());
                    Serialize<I + 1, N>(node, object);
                }

                template <std::size_t I, std::size_t N, typename T>
                inline typename std::enable_if<I == N, void>::type
                Serialize(NodeType &node, T const &object)
                {
                    Common::Unused(node);
                    Common::Unused(object);
                }

            }   // namespace Detail

            template <typename T, typename TStream>
            inline void Serialize(T const &object, TStream &stream, std::string const &root = {})
            {
                Detail::Serialize(stream, object, root);
            }

            template <typename T>
            inline Common::Buffer Serialize(T const &object, std::string const &root = {})
            {
                Common::Buffer buffer;

                {
                    boost::iostreams::filtering_ostream stream{boost::iostreams::back_inserter(buffer)};
                    Serialize(object, stream);
                    stream.flush();
                }

                return buffer;
            }

        }   // namespace Xml
    }   // namespace Serialization
}   // namespace Mif

#endif  // !__MIF_SERIALIZATION_XML_H__
