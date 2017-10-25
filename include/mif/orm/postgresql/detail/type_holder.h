//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_POSTGRESQL_DETAIL_TYPE_HOLDER_H__
#define __MIF_ORM_POSTGRESQL_DETAIL_TYPE_HOLDER_H__

// STD
#include <string>

// MIF
#include "mif/common/static_string.h"

#define MIF_ORM_POSTGRESQL_IMPL(type_, name_) \
    template <> \
    struct Holder<type_> \
    { \
        using Name = MIF_STATIC_STR(name_); \
        using Type = type_; \
    }; \

namespace Mif
{
    namespace Orm
    {
        namespace PostgreSql
        {
            namespace Detail
            {
                namespace Type
                {

                    template <typename T>
                    struct Holder
                    {
                        using Name = MIF_STATIC_STR("");
                        using Type = T;
                    };

                    MIF_ORM_POSTGRESQL_IMPL(bool, "integer")
                    MIF_ORM_POSTGRESQL_IMPL(char, "integer")
                    MIF_ORM_POSTGRESQL_IMPL(unsigned char, "integer")
                    MIF_ORM_POSTGRESQL_IMPL(short, "integer")
                    MIF_ORM_POSTGRESQL_IMPL(unsigned short, "integer")
                    MIF_ORM_POSTGRESQL_IMPL(int, "integer")
                    MIF_ORM_POSTGRESQL_IMPL(unsigned int, "integer")

                    MIF_ORM_POSTGRESQL_IMPL(long, "bigint")
                    MIF_ORM_POSTGRESQL_IMPL(unsigned long, "bigint")
                    MIF_ORM_POSTGRESQL_IMPL(long long, "bigint")
                    MIF_ORM_POSTGRESQL_IMPL(unsigned long long, "bigint")

                    MIF_ORM_POSTGRESQL_IMPL(float, "real")
                    MIF_ORM_POSTGRESQL_IMPL(double, "double precision")

                    MIF_ORM_POSTGRESQL_IMPL(std::string, "varchar")

                    using Serial = MIF_STATIC_STR("serial");
                    using BigSerial = MIF_STATIC_STR("bigserial");

                }   //namespace Type
            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Mif

#undef MIF_ORM_POSTGRESQL_IMPL

#endif  // !__MIF_ORM_POSTGRESQL_DETAIL_TYPE_HOLDER_H__
