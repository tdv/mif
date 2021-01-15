//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_DETAIL_REGISTRY_H__
#define __MIF_REMOTE_DETAIL_REGISTRY_H__

// STD
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

// MIF
#include "mif/common/detail/hierarchy.h"
#include "mif/common/index_sequence.h"

namespace Mif
{
    namespace Remote
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

                template <typename TInterface>
                struct Registry;

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
                    template <typename T, typename TPS, typename TPrm, std::size_t ... Indexes>
                    static typename T::Result CallHandler(TPrm && prm, Common::IndexSequence<Indexes ... > const *)
                    {
                        return T::template Visit<TPS>(std::get<Indexes>(prm) ... );
                    }

                    template <typename T, std::size_t I, typename TPrm>
                    static typename T::Result
                    Visit(bool &next, typename Item<I>::Index::value_type, TPrm && prm)
                    {
                        using PSType = typename Item<I>::Type::template Type<typename T::Serializer>;
                        typename T::Result result = CallHandler<T, PSType>(std::forward<TPrm>(prm),
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
    }   // namespace Remote
}   // namespace Mif

#define MIF_REMOTE_REGISTER_PS(interface_) \
    namespace Mif \
    { \
        namespace Remote \
        { \
            namespace Detail \
            { \
                namespace Registry \
                { \
                    template <> \
                    struct Registry< :: interface_> \
                    { \
                        static constexpr auto Id = Counter::GetLast(Common::Detail::FakeHierarchy{}) + 1; \
                        template <typename TSerializer> \
                        using Type = :: interface_ ## _PS <TSerializer>; \
                    }; \
                    template <> \
                    struct Item<Registry< :: interface_>::Id> \
                    { \
                        using Index = std::integral_constant<std::size_t, Registry< :: interface_>::Id>; \
                        using Type = Registry< :: interface_>; \
                    }; \
                    namespace Counter \
                    { \
                        inline constexpr std::size_t GetLast(Common::Detail::MakeHierarchy \
                            < \
                                Registry< :: interface_>::Id \
                            >) \
                        { \
                            return Registry< :: interface_> ::Id; \
                        } \
                    } \
                } \
            } \
        } \
    }

#endif  // !__MIF_REMOTE_DETAIL_REGISTRY_H__
