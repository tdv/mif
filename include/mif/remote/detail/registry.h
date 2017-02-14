//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_DETAIL_REGISTRY_H__
#define __MIF_REMOTE_DETAIL_REGISTRY_H__

// STD
#include <cstdint>
#include <type_traits>

// MIF
#include "mif/common/detail/hierarchy.h"

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
