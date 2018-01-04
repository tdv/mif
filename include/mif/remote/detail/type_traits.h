//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_DETAIL_TYPE_TRAITS_H__
#define __MIF_REMOTE_DETAIL_TYPE_TRAITS_H__

// STD
#include <type_traits>

// MIF
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Remote
    {
        namespace Detail
        {
            namespace Traits
            {

                template <typename T>
                using ExtractType = typename std::remove_const
                        <
                            typename std::remove_reference
                            <
                                typename std::remove_pointer<T>::type
                            >::type
                        >::type;

                template <typename T>
                inline constexpr bool TIsTservicePtr(Service::TIntrusivePtr<T> const *)
                {
                    static_assert(std::is_same<Service::TServicePtr<T>, boost::intrusive_ptr<T>>::value,
                        "TServicePtr should be parameterized by a type derived from IService or be IService.");
                    return true;
                }

                inline constexpr bool TIsTservicePtr(...)
                {
                    return false;
                }

                template <typename T>
                inline constexpr bool IsTServicePtr()
                {
                    return TIsTservicePtr(static_cast<T const *>(nullptr));
                }

                template <typename T>
                inline constexpr bool IsInterfaceRawPtr()
                {
                    return std::is_pointer<T>::value &&
                            (
                                std::is_base_of<Service::IService, ExtractType<T>>::value ||
                                std::is_same<Service::IService, ExtractType<T>>::value
                            );
                }

                template <typename T>
                inline constexpr bool IsInterfaceRef()
                {
                    return std::is_reference<T>::value &&
                            (
                                std::is_base_of<Service::IService, ExtractType<T>>::value ||
                                std::is_same<Service::IService, ExtractType<T>>::value
                            );
                }

                template <typename T>
                inline constexpr bool IsInterfaceSmartPtr()
                {
                    return !std::is_pointer<T>::value && IsTServicePtr<ExtractType<T>>();
                }

                template <typename T>
                inline constexpr bool IsNotInterface()
                {
                    return !std::is_base_of<Service::IService, ExtractType<T>>::value &&
                            !std::is_same<Service::IService, ExtractType<T>>::value &&
                            !IsTServicePtr<ExtractType<T>>();
                }

                template <typename T>
                inline constexpr bool IsPtrOrRef()
                {
                    return std::is_pointer<T>::value || std::is_reference<T>::value;
                }

                template <typename T>
                inline constexpr bool IsNotInterfaceValue()
                {
                    return !std::is_pointer<T>::value &&
                            !std::is_reference<T>::value &&
                            !IsTServicePtr<T>();
                }

            }   // namespace Traits
        }   // namespace Detail
    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_DETAIL_TYPE_TRAITS_H__
