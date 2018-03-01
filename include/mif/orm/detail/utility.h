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
#include <type_traits>

// MIF
#include "mif/common/static_string.h"

namespace Mif
{
    namespace Orm
    {
        namespace Detail
        {
            namespace Utility
            {

                template <typename TName, typename TFields, std::size_t I, std::size_t N, std::size_t Attempts>
                struct UniqueName final
                {
                    static_assert(Attempts <= 10, "[Mif::Orm::Detail::Utility::UniqueName] "
                            "No more than 10 attempts to create a unique name. "
                            "Correct the field names in the structure."
                        );

                    // TODO: translate names into format 'a_name_in_the_lower_case_with_underlining'
                    using Name = typename std::conditional
                        <
                            std::is_same<TName, typename TFields::template Field<I - 1>::Name>::value,
                            typename UniqueName<Common::StringCat<TName, Common::ToString<N>>, TFields, I - 1, N + 1, Attempts + 1>::Name,
                            typename UniqueName<TName, TFields, I - 1, N, Attempts>::Name
                        >::type;
                };

                template <typename TName, typename TFields, std::size_t N, std::size_t Attempts>
                struct UniqueName<TName, TFields, 0, N, Attempts> final
                {
                    using Name = TName;
                };

                template <typename TFields, typename TName>
                using CreateUniqueFieldName = typename UniqueName<TName, TFields, TFields::Count, 1, 0>::Name;

            }   // namespace Utility
        }   // namespace Detail
    }   // namespace Orm
}   // namespace Orm

#endif  // !__MIF_ORM_DETAIL_UTILITY_H__
