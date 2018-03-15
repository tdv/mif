//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_POSTGRESQL_DETAIL_FIELD_TRAITS_H__
#define __MIF_ORM_POSTGRESQL_DETAIL_FIELD_TRAITS_H__

// MIF
#include "mif/common/config.h"
#ifdef MIF_WITH_POSTGRESQL

// STD
#include <string>
#include <tuple>
#include <utility>

// MIF
#include "mif/common/detail/tuple_utility.h"
#include "mif/common/static_string.h"
#include "mif/common/unused.h"
#include "mif/orm/postgresql/detail/common.h"
#include "mif/orm/detail/table_fields.h"

namespace Mif
{
    namespace Orm
    {
        namespace PostgreSql
        {
            namespace Detail
            {
                namespace FieldTraits
                {
                    namespace Trait
                    {

                        template <typename>
                        struct ToString;

#define MIF_ORM_POSTGRESQL_FIELD_TRAIT_IMPL(trait_, text_) \
template <> \
struct ToString<Orm::Detail::FieldTraits::FieldTrait<Orm::Detail::FieldTraits::Trait_ ##trait_ >> final \
{ \
    using Name = MIF_STATIC_STR( text_ ); \
};

                        MIF_ORM_POSTGRESQL_FIELD_TRAIT_IMPL(Counter, "")
                        MIF_ORM_POSTGRESQL_FIELD_TRAIT_IMPL(NotNull, "NOT NULL")
                        MIF_ORM_POSTGRESQL_FIELD_TRAIT_IMPL(Nullable, "NULL")
                        MIF_ORM_POSTGRESQL_FIELD_TRAIT_IMPL(PrimaryKey, "PRIMARY KEY")
                        MIF_ORM_POSTGRESQL_FIELD_TRAIT_IMPL(Unique, "UNIQUE")
                        MIF_ORM_POSTGRESQL_FIELD_TRAIT_IMPL(WithTimezone, "WITH TIME ZONE")
                            MIF_ORM_POSTGRESQL_FIELD_TRAIT_IMPL(WithoutTimezone, "WITHOUT TIME ZONE")
                        MIF_ORM_POSTGRESQL_FIELD_TRAIT_IMPL(Reference, "")

#undef MIF_ORM_POSTGRESQL_FIELD_TRAIT_IMPL
                    }   // namespace Trait


                    template <typename TTrait, typename TTraits, bool = Common::Detail::TupleContains<TTrait, TTraits>::value>
                    struct AppendTrait final
                    {
                        static void Append(StringList &traits)
                        {
                            std::string trait = Trait::ToString<TTrait>::Name::Value;
                            if (!trait.empty())
                                traits.emplace_back(std::move(trait));
                        }
                    };

                    template <typename TTrait, typename TTraits>
                    struct AppendTrait<TTrait, TTraits, false> final
                    {
                        static void Append(StringList &traits)
                        {
                            Common::Unused(traits);
                        }
                    };

                    template <typename TTraits, typename TAllTraits>
                    struct TupleToStringList;

                    template <typename TTraits, typename TCur, typename ... TTail>
                    struct TupleToStringList<TTraits, std::tuple<TCur, TTail ... >> final
                    {
                        static void Get(StringList &traits)
                        {
                            AppendTrait<TCur, TTraits>::Append(traits);
                            TupleToStringList<TTraits, std::tuple<TTail ... >>::Get(traits);
                        }
                    };

                    template <typename TTraits>
                    struct TupleToStringList<TTraits, std::tuple<>> final
                    {
                        static void Get(StringList &traits)
                        {
                            Common::Unused(traits);
                        }
                    };

                    template <typename TTraits>
                    struct ToStringList final
                    {
                        static void Get(StringList &traits)
                        {
                            TupleToStringList<TTraits, Orm::Detail::FieldTraits::AllTypeTraits>::Get(traits);
                        }
                    };

                }   // namespace FieldTraits
            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Mif

#endif  // !MIF_WITH_POSTGRESQL
#endif  // !__MIF_ORM_POSTGRESQL_DETAIL_FIELD_TRAITS_H__
