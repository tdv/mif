//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_POSTGRESQL_DETAIL_MODIFICATIONS_H__
#define __MIF_ORM_POSTGRESQL_DETAIL_MODIFICATIONS_H__

// MIF
#include "mif/common/config.h"
#ifdef MIF_WITH_POSTGRESQL

// BOOST
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

// MIF
#include "mif/common/unused.h"
#include "mif/reflection/reflection.h"
#include "mif/orm/detail/utility.h"
#include "mif/orm/postgresql/detail/common.h"
#include "mif/orm/postgresql/detail/utility.h"
#include "mif/reflection/reflection.h"
#include "mif/serialization/traits.h"

namespace Mif
{
    namespace Orm
    {
        namespace PostgreSql
        {
            namespace Detail
            {

                template <typename TSchema>
                class Inserter final
                {
                public:
                    template <typename T>
                    static typename std::enable_if
                        <
                            Reflection::IsReflectable<typename std::decay<T>::type>() &&
                                !std::is_enum<typename std::decay<T>::type>::value,
                            void
                        >::type
                    Insert(T const &value, StringList &items)
                    {
                        using Type = typename std::decay<T>::type;
                        using Entity = typename Orm::Detail::FindTableByType<Type, typename Schema::Entities>::Entity;
                        using EntityType = typename Entity::EntityType;
                        using Meta = Reflection::Reflect<EntityType>;

                        std::string const tableName = Utility::EntityName<typename Schema::Name, typename Meta::Name>::Create();
                        StringList fields;
                        CreateFieldsList<typename Meta::Fields, Meta::Fields::Count>(fields);
                        if (!fields.empty())
                        {
                            std::string sql = "INSERT INTO " + tableName + " (";
                            sql += boost::algorithm::join(fields, ", ");
                            sql += ")\n";
                            sql += "VALUES (";
                            sql += ");\n";

                            items.emplace_back(std::move(sql));
                        }
                        else
                        {
                            // TODO: returning foreign key
                        }
                    }

                private:
                    using Schema = Orm::Detail::FromSchema<TSchema>;

                    template <typename TField>
                    static typename std::enable_if
                        <
                            Serialization::Traits::IsSimple<typename TField::Type>() ||
                                std::is_same<typename TField::Type, boost::posix_time::ptime>::value ||
                                std::is_same<typename TField::Type, boost::posix_time::ptime::date_type>::value ||
                                std::is_enum<typename TField::Type>::value,
                            void
                        >::type
                    AddSimpleField(StringList &list)
                    {
                        list.emplace_back(Utility::QuoteReserved(TField::Name::Value));
                    }

                    template <typename TField>
                    static typename std::enable_if
                        <
                            Serialization::Traits::IsIterable<typename TField::Type>() ||
                                (!std::is_enum<typename TField::Type>::value && Reflection::IsReflectable<typename TField::Type>()),
                            void
                        >::type
                    AddSimpleField(StringList &list)
                    {
                        Common::Unused(list);
                    }

                    template <typename TFields, std::size_t I>
                    static typename std::enable_if<I != 0 ,void>::type CreateFieldsList(StringList &list)
                    {
                        using Field = typename TFields::template Field<TFields::Count - I>;
                        AddSimpleField<Field>(list);
                        CreateFieldsList<TFields, I - 1>(list);
                    }

                    template <typename TFields, std::size_t I>
                    static typename std::enable_if<I == 0 ,void>::type CreateFieldsList(StringList &list)
                    {
                        Common::Unused(list);
                    }
                };

            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Orm

#endif  // !MIF_WITH_POSTGRESQL
#endif  // !__MIF_ORM_POSTGRESQL_DETAIL_MODIFICATIONS_H__
