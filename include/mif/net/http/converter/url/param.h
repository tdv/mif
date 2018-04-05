//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_CONVERTER_URL_PARAM_H__
#define __MIF_NET_HTTP_CONVERTER_URL_PARAM_H__

// STD
#include <algorithm>
#include <functional>
#include <iterator>
#include <set>
#include <string>
#include <sstream>
#include <type_traits>

// BOOST
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// MIF
#include "mif/common/static_string.h"
#include "mif/reflection/reflection.h"
#include "mif/serialization/traits.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Converter
            {
                namespace Url
                {

                    struct Param final
                    {
                        template <typename T>
                        static typename std::enable_if<std::is_same<T, std::string>::value, T>::type
                        Convert(std::string const &value)
                        {
                            return value;
                        }

                        template <typename T>
                        static typename std::enable_if<std::is_integral<T>::value, T>::type
                        Convert(std::string const &value)
                        {
                            return static_cast<T>(std::stoll(value));
                        }

                        template <typename T>
                        static typename std::enable_if<std::is_floating_point<T>::value, T>::type
                        Convert(std::string const &value)
                        {
                            return static_cast<T>(std::stod(value));
                        }

                        template <typename T>
                        static typename std::enable_if<Serialization::Traits::IsIterable<T>(), T>::type
                        Convert(std::string const &value)
                        {
                            std::set<std::string> items;
                            boost::split(items, value, boost::is_any_of(";"));
                            T seq;
                            std::transform(std::begin(items), std::end(items), std::inserter(seq, std::begin(seq)),
                                    std::ptr_fun(Param::template Convert<typename T::value_type>));
                            return seq;
                        }

                        template <typename T>
                        static typename std::enable_if<std::is_same<T, boost::posix_time::ptime::date_type>::value, T>::type
                        Convert(std::string const &value)
                        {
                            std::istringstream stream{value};
                            stream.imbue(std::locale(std::locale::classic(),
                                    new boost::posix_time::time_input_facet("%Y.%m.%d")));
                            boost::posix_time::ptime pt;
                            stream >> pt;
                            return pt.date();
                        }

                        template <typename T>
                        static typename std::enable_if<std::is_same<T, boost::posix_time::ptime::time_duration_type>::value, T>::type
                        Convert(std::string const &value)
                        {
                            std::istringstream stream{value};
                            stream.imbue(std::locale(std::locale::classic(),
                                    new boost::posix_time::time_input_facet("%H:%M:%S")));
                            boost::posix_time::ptime pt;
                            stream >> pt;
                            return pt.time_of_day();
                        }

                        template <typename T>
                        static typename std::enable_if<std::is_same<T, boost::posix_time::ptime>::value, T>::type
                        Convert(std::string const &value)
                        {
                            std::istringstream stream{value};
                            stream.imbue(std::locale(std::locale::classic(),
                                    new boost::posix_time::time_input_facet("%Y.%m.%d %H:%M:%S")));
                            boost::posix_time::ptime pt;
                            stream >> pt;
                            return pt;
                        }

                        template <typename T>
                        static typename std::enable_if<Reflection::IsReflectable<T>() && std::is_enum<T>::value, T>::type
                        Convert(std::string const &value)
                        {
                            return Reflection::FromString<T>(value);
                        }
                    };

                }   // namespace Url
            }   // namespace Converter
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif


#endif  // !__MIF_NET_HTTP_CONVERTER_URL_PARAM_H__
