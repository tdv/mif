//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_POSTGRESQL_DETAIL_SIMPLE_TYPES_H__
#define __MIF_ORM_POSTGRESQL_DETAIL_SIMPLE_TYPES_H__

// MIF
#include "mif/common/config.h"
#ifdef MIF_WITH_POSTGRESQL

// STD
#include <string>
#include <type_traits>

// MIF
#include "mif/common/static_string.h"

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
                    namespace Simple
                    {

                        template <typename>
                        struct Holder;

#define MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(type_, name_) \
    template <> \
    struct Holder<type_> \
    { \
        using Name = MIF_STATIC_STR(name_); \
        using Type = type_; \
    };
                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(bool, "INTEGERU")
                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(char, "INTEGER")
                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(unsigned char, "INTEGER")
                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(short, "INTEGER")
                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(unsigned short, "INTEGER")
                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(int, "INTEGER")
                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(unsigned int, "INTEGER")

                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(long, "BIGINT")
                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(unsigned long, "BIGINT")
                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(long long, "BIGINT")
                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(unsigned long long, "BIGINT")

                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(float, "REAL")
                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(double, "DOUBLE PRECISION")

                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(std::string, "TEXT")

                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(boost::posix_time::ptime::date_type, "DATE")
                        MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL(boost::posix_time::ptime, "TIMESTAMP")

                        using Serial = MIF_STATIC_STR("SERIAL");
                        using BigSerial = MIF_STATIC_STR("BIGSERIAL");

#undef MIF_ORM_POSTGRESQL_SIMPLE_TYPE_IMPL

                        struct TypeName final
                        {
                            template <typename T>
                            static typename std::enable_if<!std::is_enum<T>::value, std::string>::type Get()
                            {
                                return Holder<T>::Name::Value;
                            }

                            template <typename T>
                            static typename std::enable_if<std::is_enum<T>::value, std::string>::type Get()
                            {
                                return Holder<typename std::underlying_type<T>::type>::Name::Value;
                            }
                        };

                    }   // namespace Simple
                }   // namespace Type
            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Mif

#endif  // !MIF_WITH_POSTGRESQL
#endif  // !__MIF_ORM_POSTGRESQL_DETAIL_SIMPLE_TYPES_H__
