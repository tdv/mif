//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REFLECTION_REFLECT_TYPE_H__
#define __MIF_REFLECTION_REFLECT_TYPE_H__

// STD
#include <tuple>

// MIF
#include "mif/common/static_string.h"
#include "mif/common/detail/hierarchy.h"
#include "mif/reflection/detail/registry.h"

namespace Mif
{
    namespace Reflection
    {
        namespace Detail
        {

            template <typename>
            struct Class;

        }   // namespace Detail
    }   // namespace Reflection
}   // namespace Mif

#define MIF_REFLECT_BEGIN(type_, ... ) \
    class type_ ## _MetaInfo final \
    { \
    private: \
        template <typename> \
        friend class ::Mif::Reflection::Detail::Class; \
        template <typename> \
        friend class ::Mif::Reflection::Detail::FieldsList; \
        MIF_DECLARE_SRTING_PROVIDER(TypeNameProvider, #type_) \
        using ClassType = type_; \
        using BaseTypes = std::tuple<__VA_ARGS__>; \
        static char (&GetNextCounter(void *))[1];

#define MIF_REFLECT_FIELD(field_) \
    struct field_ ## _MetaInfo \
    { \
        MIF_DECLARE_SRTING_PROVIDER(TypeNameProvider, #field_) \
        using FieldType = decltype(ClassType::field_); \
        static FieldType ClassType::* Access() \
        { \
            return &ClassType :: field_ ; \
        } \
    }; \
    enum { field_ ## _Index = sizeof(GetNextCounter(static_cast<::Mif::Common::Detail::FakeHierarchy *>(nullptr))) }; \
    static char (&GetNextCounter(::Mif::Common::Detail::Hierarchy<field_ ## _Index> *))[field_ ## _Index + 1]; \
    static field_ ## _MetaInfo GetFieldInfo(::Mif::Common::Detail::Hierarchy<field_ ## _Index - 1>);

#define MIF_REFLECT_END() \
        enum { FieldsCount = sizeof(GetNextCounter(static_cast<::Mif::Common::Detail::FakeHierarchy *>(nullptr))) }; \
    };

#endif  // !__MIF_REFLECTION_REFLECT_TYPE_H__
