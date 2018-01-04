//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     04.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REFLECTION_DETAIL_REGISTRY_H__
#define __MIF_REFLECTION_DETAIL_REGISTRY_H__

// STD
#include <cstdint>
#include <type_traits>

// MIF
#include "mif/common/detail/hierarchy.h"
#include "mif/common/static_string.h"

namespace Mif
{
    namespace Reflection
    {
        namespace Detail
        {
            namespace Registry
            {
                namespace Counter
                {
                    inline constexpr std::size_t GetLast(...)
                    {
                        return 0;
                    }

                }   // namespace Counter

                struct None;

                template <typename Key>
                struct Registry
                {
                    using Type = None;
                };

                template <std::size_t I>
                struct Item;

                class Visitor final
                {
                public:
                    template <typename T, typename ... TPrm>
                    static typename T::Result Accept(TPrm && ... prm)
                    {
                        bool next = false;
                        return Visit<T, 1>(next, std::move(std::make_tuple(std::forward<TPrm>(prm) ... )));
                    }

                private:
                    template <typename T, typename Type, typename TPrm, std::size_t ... Indexes>
                    static typename T::Result CallHandler(TPrm && prm, Common::IndexSequence<Indexes ... > const *)
                    {
                        return T::template Visit<Type>(std::get<Indexes>(prm) ... );
                    }

                    template <typename T, std::size_t I, typename TPrm>
                    static typename T::Result
                    Visit(bool &next, typename Item<I>::Index::value_type, TPrm && prm)
                    {
                        using Type = typename Item<I>::Type::Key;
                        typename T::Result result = CallHandler<T, Type>(std::forward<TPrm>(prm),
                            static_cast<Common::MakeIndexSequence<std::tuple_size<TPrm>::value> const *>(nullptr));
                        next = !result;
                        return result;
                    }

                    template <typename T, std::size_t I>
                    static typename std::enable_if
                        <
                            I != Common::Detail::FakeHierarchyLength::value,
                            typename T::Result
                        >::type
                    Visit(bool &next, ...)
                    {
                        next = false;
                        return {};
                    }

                    template <typename T, std::size_t I, typename TPrm>
                    static typename std::enable_if
                        <
                            I == Common::Detail::FakeHierarchyLength::value,
                            typename T::Result
                        >::type
                    Visit(bool &next, TPrm &&)
                    {
                        next = false;
                        return {};
                    }

                    template <typename T, std::size_t I, typename TPrm>
                    static typename std::enable_if
                        <
                            I != Common::Detail::FakeHierarchyLength::value,
                            typename T::Result
                        >::type
                    Visit(bool &next, TPrm && prm)
                    {
                        auto res = Visit<T, I>(next, std::size_t{}, std::forward<TPrm>(prm));
                        if (next)
                            res = Visit<T, I + 1>(next, std::forward<TPrm>(prm));
                        return res;
                    }
                };

            }   // namespace Registry
        }   // namespace Detail
    }   // namespace Reflection
}   // namespace Mif

#ifndef MIF_BOOST_TYPE_SERIALIZER
    #define MIF_BOOST_TYPE_SERIALIZER(type_)
#endif  // !MIF_BOOST_TYPE_SERIALIZER

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
                    struct Registry<type_ ## _MetaInfo :: ClassType> \
                    { \
                        static constexpr auto Id = Counter::GetLast(Common::Detail::FakeHierarchy{}) + 1; \
                        using Type = type_ ## _MetaInfo; \
                        using Key = type_ ## _MetaInfo :: ClassType; \
                        using TypeFullNameProvider = MIF_STATIC_STR( #type_ ); \
                    }; \
                    template <> \
                    struct Item<Registry<type_ ## _MetaInfo :: ClassType>::Id> \
                    { \
                        using Index = std::integral_constant<std::size_t, Registry<type_ ## _MetaInfo :: ClassType>::Id>; \
                        using Type = Registry<type_ ## _MetaInfo :: ClassType>; \
                    }; \
                    namespace Counter \
                    { \
                        inline constexpr std::size_t GetLast(Common::Detail::MakeHierarchy \
                            < \
                                Registry<type_ ## _MetaInfo :: ClassType>::Id \
                            >) \
                        { \
                            return Registry<type_ ## _MetaInfo :: ClassType>::Id; \
                        } \
                    } \
                } \
            } \
        } \
    } \
    MIF_BOOST_TYPE_SERIALIZER(type_ ## _MetaInfo :: ClassType)

#endif  // !__MIF_REFLECTION_DETAIL_REGISTRY_H__
