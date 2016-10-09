//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REFLECTION_REFLECT_TYPE_H__
#define __MIF_REFLECTION_REFLECT_TYPE_H__

namespace Mif
{
    namespace Reflection
    {
        namespace Detail
        {

            template <typename>
            struct Class;

            namespace Registry
            {

                struct None;

                template <typename Key>
                struct Registry
                {
                    using Type = None;
                };

            }   // namespace Registry
        }   // namespace Detail
    }   // namespace Reflection
}   // namespace Mif

#define MIF_DECLARE_SRTING_PROVIDER(name_, value_) \
    struct name_ \
    { \
        static constexpr auto Value = value_; \
    };

#define MIF_REFLECT_BEGIN(type_, ... ) \
    class type_ ## _MetaInfo final \
    { \
    private: \
        template <typename> \
        friend class ::Mif::Reflection::Detail::Class; \
        MIF_DECLARE_SRTING_PROVIDER(TypeNameProvider, #type_) \
        using Type = type_;

#define MIF_REFLECT_FIELD(field_)

#define MIF_REFLECT_END() \
    };

#define MIF_REGISTER_REFLECTED_TYPE(type_) \
    namespace Mif \
    { \
        namespace Reflection \
        { \
            namespace Detail \
            { \
                namespace Registry \
                { \
                    template <> \
                    struct Registry<type_> \
                    { \
                        using Type = type_ ## _MetaInfo; \
                        using Key = type_; \
                    }; \
                } \
            } \
        } \
    }


#endif  // !__MIF_REFLECTION_REFLECT_TYPE_H__
