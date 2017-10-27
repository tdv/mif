//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_DETAIL_UTILITY_H__
#define __MIF_ORM_DETAIL_UTILITY_H__

// STD
#include <cstdint>
#include <set>
#include <string>
#include <tuple>

// MIF
#include "mif/orm/detail/entity_info.h"

namespace Mif
{
    namespace Orm
    {
        namespace Detail
        {
            namespace Utility
            {
                struct Empty;

                namespace Detail
                {

                    template <typename TEntities, std::size_t I>
                    struct EntityNameEnum
                    {
                        static void Get(std::set<std::string> &names)
                        {
                            using Name = typename std::tuple_element<I - 1, TEntities>::type::Name;
                            names.insert(Name::Value);
                            EntityNameEnum<TEntities, I - 1>::Get(names);
                        }
                    };

                    template <typename TEntities>
                    struct EntityNameEnum<TEntities, 0>
                    {
                        static void Get(std::set<std::string> &names)
                        {
                            Common::Unused(names);
                        }
                    };

                    template <typename T, typename TItems, std::size_t I, typename TItem>
                    struct SelectTableItem
                    {
                        using Item = typename SelectTableItem<T, TItems, I - 1, typename std::tuple_element<I - 1, TItems>::type>::Item;
                    };

                    template <typename T, typename TItems, std::size_t I, typename TEntity, typename ... TTraits>
                    struct SelectTableItem<T, TItems, I, TableInfo<TEntity, TTraits ... >>
                    {
                        using Item = TableInfo<TEntity, TTraits ... >;
                    };

                    template <typename T, typename TItems, typename TItem>
                    struct SelectTableItem<T, TItems, 0, TItem>
                    {
                        using Item = Empty;
                    };

                }   // namespace Detail

                template <typename TEntities>
                inline std::set<std::string> CreateNameSet()
                {
                    std::set<std::string> names;
                    Detail::EntityNameEnum<TEntities, std::tuple_size<TEntities>::value>::Get(names);
                    return names;
                }

                template <typename T, typename TItems>
                using SelectTableItem = typename Detail::SelectTableItem
                        <
                            T,
                            TItems,
                            std::tuple_size<TItems>::value,
                            Empty
                        >::Item;

            }   // namespace Utility
        }   // namespace Detail
    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_DETAIL_UTILITY_H__
