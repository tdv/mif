//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_DETAIL_UTILITY_H__
#define __MIF_ORM_DETAIL_UTILITY_H__

// STD
#include <cstdint>
#include <tuple>
#include <type_traits>

// MIF
#include "mif/common/static_string.h"
#include "mif/orm/detail/entity.h"
#include "mif/orm/forward.h"

namespace Mif
{
    namespace Orm
    {
        namespace Detail
        {

            template <typename>
            struct FromSchema;

            template <char ... TName, typename ... TEntities>
            struct FromSchema<Entity<Schema<Common::StaticString<TName ... >, Entity<TEntities> ... >>> final
            {
                using Name = Common::StaticString<TName ... >;
                using Entities = std::tuple<TEntities ... >;
            };

            template <typename T, typename TEntities, std::size_t I = std::tuple_size<TEntities>::value>
            struct FindTableByType final
            {
                using Entity = typename std::conditional
                    <
                        std::is_same<T, typename std::tuple_element<I - 1, TEntities>::type::EntityType>::value,
                        typename std::tuple_element<I - 1, TEntities>::type,
                        typename FindTableByType<T, TEntities, I - 1>::Entity
                    >::type;
            };

            template <typename T, typename TEntities>
            struct FindTableByType<T, TEntities, 0>
            {
                using Entity = Table<T>;
            };

        }   // namespace Detail
    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_DETAIL_UTILITY_H__
