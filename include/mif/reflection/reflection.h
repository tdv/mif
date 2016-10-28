//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REFLECTION_REFLECTION_H__
#define __MIF_REFLECTION_REFLECTION_H__

// STD
#include <cstdint>

// MIF
#include "mif/common/static_string.h"
#include "mif/common/detail/hierarchy.h"

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
                using Name = Common::MakeStaticString<typename T::TypeNameProvider>;
                using Type = typename T::FieldType;
                static auto Access() -> decltype(T::Access())
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
                using FullName = Common::MakeStaticString<typename RegItemType::TypeFullNameProvider>;
                using Name = Common::MakeStaticString<typename MetaType::TypeNameProvider>;
                using Base = typename MetaType::BaseTypes;
                using Type = typename MetaType::ClassType;
                using Fields = FieldsList<T>;
            };

        }   // namespace Detail

        template <typename T>
        using Reflect = Detail::Class<T>;

    }   // namespace Reflection
}   // namespace Mif

#endif  // !__MIF_REFLECTION_REFLECTION_H__
