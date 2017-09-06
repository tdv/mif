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

                    MIF_DECLARE_SRTING_PROVIDER(Id, "id")
                    MIF_DECLARE_SRTING_PROVIDER(Value, "val")

                }   // namespace Tag

                class Serializer final
                {
                public:
                    template <typename T>
                    void Serialize(std::ostream &stream, T const &object)
                    {
                        pugi::xml_document doc;

                        Serialize(doc, object);

                        doc.save(stream);
                    }

                private:
                    template <typename T>
                    typename std::enable_if<std::is_pointer<T>::value, void>::type
                    Serialize(pugi::xml_node &node, T const &)
                    {
                        static_assert(!std::is_pointer<T>::value, "[Mif::Serialization::Xml::Detail::Serializer] You can't serialize the raw pointers.");
                    }

                    template <typename T>
                    typename std::enable_if<!Reflection::IsReflectable<T>() && std::is_enum<T>::value, void>::type
                    Serialize(pugi::xml_node &node, T const &object)
                    {
                    }

                    template <typename T>
                    typename std::enable_if<Reflection::IsReflectable<T>() && std::is_enum<T>::value, void>::type
                    Serialize(pugi::xml_node &node, T const &object)
                    {
                    }

                    template <typename T>
                    typename std::enable_if<Traits::IsSimple<T>(), void>::type
                    Serialize(pugi::xml_node &node, T const &object)
                    {
                    }

                    template <typename T>
                    typename std::enable_if<Reflection::IsReflectable<T>() && !std::is_enum<T>::value, void>::type
                    Serialize(pugi::xml_node &node, T const &object)
                    {
                    }

                    template <typename T>
                    typename std::enable_if<Traits::IsSmartPointer<T>(), void>::type
                    Serialize(pugi::xml_node &node, T const &ptr)
                    {
                    }

                    template <typename T>
                    typename std::enable_if<Traits::IsIterable<T>(), void>::type
                    Serialize(pugi::xml_node &node, T const &array)
                    {
                    }

                    template <typename ... T>
                    void Serialize(pugi::xml_node &node, std::tuple<T ... > const &tuple)
                    {
                    }

                    template <typename TFirst, typename TSecond>
                    void Serialize(pugi::xml_node &node, std::pair<TFirst, TSecond> const &pair)
                    {
                    }
                };

            }   // namespace Detail

            // TODO:

        }   // namespace Xml
    }   // namespace Serialization
}   // namespace Mif

#endif  // !__MIF_SERIALIZATION_XML_H__
