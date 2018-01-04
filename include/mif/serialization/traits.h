//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERIALIZATION_TRAITS_H__
#define __MIF_SERIALIZATION_TRAITS_H__

// STD
#include <string>
#include <type_traits>

namespace Mif
{
    namespace Serialization
    {
        namespace Traits
        {
            namespace Detail
            {

                template <typename T>
                inline std::true_type IsIterable(T const *,
                        typename T::iterator = typename std::decay<T>::type{}.begin(),
                        typename T::iterator = typename std::decay<T>::type{}.end(),
                        typename T::const_iterator = typename std::decay<T>::type{}.cbegin(),
                        typename T::const_iterator = typename std::decay<T>::type{}.cend(),
                        typename T::value_type = * typename std::decay<T>::type{}.begin()
                    );

                inline std::false_type IsIterable(...);

                template <typename T>
                inline std::true_type IsSmartPointer(T const *,
                        typename T::element_type* = typename std::decay<T>::type{}.operator -> (),
                        typename T::element_type = * typename std::decay<T>::type{}.get()
                    );

                inline std::false_type IsSmartPointer(...);

            }   // namespace Detail

            template <typename T>
            inline constexpr bool IsIterable()
            {
                return std::is_same<decltype(Detail::IsIterable(static_cast<typename std::decay<T>::type const *>(nullptr))), std::true_type>::value
                    && !std::is_same<typename std::decay<T>::type, std::string>::value;
            }

            template <typename T>
            inline constexpr bool IsSmartPointer()
            {
                return std::is_same<decltype(Detail::IsSmartPointer(static_cast<T const *>(nullptr))), std::true_type>::value;
            }

            template <typename T>
            inline constexpr bool IsSimple()
            {
                return std::is_integral<typename std::decay<T>::type>::value ||
                    std::is_floating_point<typename std::decay<T>::type>::value ||
                    std::is_same<typename std::decay<T>::type, std::string>::value;
            }

        }   // namespace Traits
    }   // namespace Serialization
}   // namespace Mif

#endif  // !__MIF_SERIALIZATION_TRAITS_H__
