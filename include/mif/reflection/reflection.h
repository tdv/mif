//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REFLECTION_REFLECTION_H__
#define __MIF_REFLECTION_REFLECTION_H__

// STD
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>

// MIF
#include "mif/common/detail/hierarchy.h"
#include "mif/common/unused.h"
#include "mif/reflection/reflect_type.h"

#define MIF_FIELD_META(field_ptr_) \
    ::Mif::Reflection::Reflect \
    < \
        typename ::Mif::Reflection::Detail::FieldOwnerType \
        < \
            decltype(field_ptr_) \
        >::Type \
        >::Fields::Field \
        < \
            ::Mif::Reflection::Detail::FieldIndex<decltype(field_ptr_), field_ptr_>::Get() \
        >

namespace Mif
{
    namespace Reflection
    {
        namespace Detail
        {

            struct Base
            {
            };

            template <typename T>
            struct FieldItem
            {
                using Name = typename T::TypeNameProvider;
                using Type = typename T::FieldType;
                static constexpr auto Access() -> decltype(T::Access())
                {
                    return T::Access();
                }
            };

            template <typename T>
            class FieldsList
            {
            private:
                using RegItemType = Registry::Registry<T>;
                using MetaType = typename RegItemType::Type;

            public:
                static constexpr std::size_t Count = MetaType::FieldsCount - 1;
                template <std::size_t Index>
                using Field = FieldItem<decltype(MetaType::GetFieldInfo(Common::Detail::Hierarchy<Index>{}))>;
            };

            template <typename T>
            class Class
            {
            private:
                using RegItemType = Registry::Registry<T>;
                using MetaType = typename RegItemType::Type;

            public:
                using FullName = typename RegItemType::TypeFullNameProvider;
                using Name = typename MetaType::TypeNameProvider;
                using Base = typename MetaType::BaseTypes;
                using Type = typename MetaType::ClassType;
                using Fields = FieldsList<T>;
            };

            template <typename T, typename TItems, std::size_t Index>
            inline typename std::enable_if<Index == 0, std::string>::type
            EnumValueToString(T const &value)
            {
                Common::Unused(value);
                throw std::invalid_argument{"[Mif::Reflection::Detail::EnumValueToString] Failed to get name for enum value \"" +
                        std::to_string(static_cast<typename std::underlying_type<T>::type>(value)) + "\""};
            }

            template <typename T, typename TItems, std::size_t Index>
            inline typename std::enable_if<Index != 0, std::string>::type
            EnumValueToString(T const &value)
            {
                using Item = typename TItems::template Field<Index - 1>;
                if (Item::Access() == value)
                    return Item::Name::Value;
                return EnumValueToString<T, TItems, Index - 1>(value);
            }

            template <typename T, typename TItems, std::size_t Index>
            inline typename std::enable_if<Index == 0, T>::type
            StringToEnumValue(std::string const &str)
            {
                Common::Unused(str);
                throw std::invalid_argument{"[Mif::Reflection::Detail::StringToEnumValue] Failed to get value from string \"" + str + "\"."};
            }

            template <typename T, typename TItems, std::size_t Index>
            inline typename std::enable_if<Index != 0, T>::type
            StringToEnumValue(std::string const &str)
            {
                using Item = typename TItems::template Field<Index - 1>;
                if (str == Item::Name::Value)
                    return Item::Access();
                return StringToEnumValue<T, TItems, Index - 1>(str);
            }

        }   // namespace Detail

        template <typename T>
        using Reflect = Detail::Class<T>;

        template <typename T>
        inline constexpr bool IsReflectable()
        {
            return !std::is_same<typename Detail::Registry::Registry<T>::Type, Detail::Registry::None>::value;
        }

        template <typename T>
        inline typename std::enable_if
            <
                std::is_enum<T>::value && IsReflectable<T>(),
                std::string
            >::type
        ToString(T const &value)
        {
            using Items = typename Reflect<T>::Fields;
            return Detail::EnumValueToString<T, Items, Items::Count>(value);
        }

        template <typename T>
        inline typename std::enable_if
            <
                std::is_enum<T>::value && IsReflectable<T>(),
                T
            >::type
        FromString(std::string const &value)
        {
            using Items = typename Reflect<T>::Fields;
            return Detail::StringToEnumValue<T, Items, Items::Count>(value);
        }

    }   // namespace Reflection
}   // namespace Mif

#endif  // !__MIF_REFLECTION_REFLECTION_H__
