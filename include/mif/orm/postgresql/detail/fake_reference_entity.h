//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_POSTGRESQL_DETAIL_FAKE_REFERENCE_ENTITY_H__
#define __MIF_ORM_POSTGRESQL_DETAIL_FAKE_REFERENCE_ENTITY_H__

// MIF
#include "mif/common/config.h"
#ifdef MIF_WITH_POSTGRESQL

// STD
#include <cstdint>

// MIF
#include "mif/reflection/reflect_type.h"

namespace Mif
{
    namespace Orm
    {
        namespace PostgreSql
        {
            namespace Detail
            {

                struct Reference
                {
                    std::size_t pkReferenceId;
                };

                namespace Meta
                {

                    MIF_REFLECT_BEGIN(Reference)
                        MIF_REFLECT_FIELD(pkReferenceId)
                    MIF_REFLECT_END()

                }   // namespace Meta
            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Mif

MIF_REGISTER_REFLECTED_TYPE(Mif::Orm::PostgreSql::Detail::Meta::Reference)

#endif  // !MIF_WITH_POSTGRESQL
#endif  // !__MIF_ORM_POSTGRESQL_DETAIL_FAKE_REFERENCE_ENTITY_H__
