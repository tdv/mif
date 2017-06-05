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
#include <type_traits>

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
        static_assert(std::is_class<ClassType>::value || std::is_enum<ClassType>::value, \
                "The type to reflect must be a class or an enum."); \
        using BaseTypes = std::tuple<__VA_ARGS__>; \
        static_assert(std::is_class<ClassType>::value || \
                (std::is_enum<ClassType>::value && !std::tuple_size<BaseTypes>::value), \
                "An enum type must not have a base classes."); \
        static char (&GetNextCounter(void *))[1];

#define MIF_REFLECT_FIELD(field_) \
    template <typename TClass> \
    struct field_ ## _FieldTypeMetaInfo final \
    { \
        MIF_DECLARE_SRTING_PROVIDER(TypeNameProvider, #field_) \
        using FieldType = decltype(TClass::field_); \
        template<typename U = TClass, class = typename std::enable_if<!std::is_enum<U>::value, U>::type> \
        static FieldType U ::* Access() \
        { \
            return & U :: field_ ; \
        } \
    }; \
    template <typename TClass> \
    static typename std::enable_if<!std::is_enum<TClass>::value, field_ ## _FieldTypeMetaInfo <TClass>>::type \
        field_ ## _GetTypeMetaInfo(); \
    template <typename TClass> \
    struct field_ ## _ItemTypeMetaInfo final \
    { \
        MIF_DECLARE_SRTING_PROVIDER(TypeNameProvider, #field_) \
        using FieldType = ClassType; \
        template<typename U = TClass, class = typename std::enable_if<std::is_enum<U>::value, U>::type> \
        static constexpr FieldType Access() \
        { \
            return U :: field_; \
        } \
    }; \
    template <typename TClass> \
    static typename std::enable_if<std::is_enum<TClass>::value, field_ ## _ItemTypeMetaInfo <TClass>>::type \
        field_ ## _GetTypeMetaInfo(); \
    using field_ ## _MetaInfo = decltype(field_ ## _GetTypeMetaInfo <ClassType> ()); \
    enum { field_ ## _Index = sizeof(GetNextCounter(static_cast<::Mif::Common::Detail::FakeHierarchy *>(nullptr))) }; \
    static char (&GetNextCounter(::Mif::Common::Detail::Hierarchy<field_ ## _Index> *))[field_ ## _Index + 1]; \
    static field_ ## _MetaInfo GetFieldInfo(::Mif::Common::Detail::Hierarchy<field_ ## _Index - 1>);

#define MIF_REFLECT_END() \
        enum { FieldsCount = sizeof(GetNextCounter(static_cast<::Mif::Common::Detail::FakeHierarchy *>(nullptr))) }; \
    };

#endif  // !__MIF_REFLECTION_REFLECT_TYPE_H__
