//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_POSTGRESQL_DETAIL_STRUCTURE_H__
#define __MIF_ORM_POSTGRESQL_DETAIL_STRUCTURE_H__

// MIF
#include "mif/common/config.h"
#ifdef MIF_WITH_POSTGRESQL

// STD
#include <cstdint>
#include <string>
#include <tuple>
#include <type_traits>

// BOOST
#include <boost/algorithm/string.hpp>

// MIF
#include "mif/common/unused.h"
#include "mif/orm/detail/entity.h"
#include "mif/orm/detail/traits_utility.h"
#include "mif/orm/forward.h"
#include "mif/orm/postgresql/detail/common.h"
#include "mif/orm/postgresql/detail/field_traits.h"
#include "mif/orm/postgresql/detail/fake_reference_entity.h"
#include "mif/orm/postgresql/detail/simple_types.h"
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
                template <typename ... T>
                class Entity final
                {
                public:
                    static void Create(StringList &items)
                    {
                        Common::Unused(items);
                    }

                private:
                    template <char const ... Ch>
                    static constexpr std::true_type IsName(Common::StaticString<Ch ... > const *);
                    static constexpr std::false_type IsName(...);

                    /*static_assert(sizeof ... (T) == 2 &&
                            decltype(IsName(static_cast<typename std::tuple_element<0, std::tuple<T ... >>::type const *>(nullptr)))::value,
                            "[Mif::Orm::PostgreSql::Detail::Entity] Unknown object to generate sql."
                        );*/
                };

                template <typename TCreated, typename TSchemaName, typename TEntity, typename ... TEntities>
                class Entity<TCreated, TSchemaName, Orm::Detail::Entity<Orm::Enum<TEntity>>, TEntities ... > final
                {
                public:
                    static void Create(StringList &items)
                    {
                        auto sql = "CREATE TYPE " + Utility::EntityName<TSchemaName, typename Meta::Name>::Create() + " AS ENUM\n";
                        sql += "(\n";
                        StringList enumItems;
                        CreateItems<Meta::Fields::Count>(enumItems);
                        sql += boost::algorithm::join(enumItems, ",\n");
                        if (!enumItems.empty())
                            sql += "\n";
                        sql += ");\n";
                        items.emplace_back(sql);
                        using CreatedEntities = typename Common::Detail::TupleCat<TCreated, std::tuple<Orm::Enum<TEntity>>>::Tuple;
                        Entity<CreatedEntities, TSchemaName, TEntities ... >::Create(items);
                    }

                private:
                    using Meta = Reflection::Reflect<TEntity>;

                    template <std::size_t I>
                    static typename std::enable_if<I == 0, void>::type
                    CreateItems(StringList &items)
                    {
                        Common::Unused(items);
                    }

                    template <std::size_t I>
                    static typename std::enable_if<I != 0, void>::type
                    CreateItems(StringList &items)
                    {
                        using Field = typename Meta::Fields::template Field<Meta::Fields::Count - I>;
                        items.emplace_back(Indent::Value + Utility::Quote(Field::Name::Value));
                        CreateItems<I - 1>(items);
                    }
                };

                template <typename TCreated, typename TSchemaName, typename TEntity, typename ... TTableTraits, typename ... TEntities>
                class Entity<TCreated, TSchemaName, Orm::Detail::Entity<Orm::Table<TEntity, TTableTraits ... >>, TEntities ... > final
                {
                public:
                    static void Create(StringList &items)
                    {
                        Context context;
                        context.name = Utility::EntityName<TSchemaName, typename Meta::Name>::Create();
                        Create(context);
                        std::copy(std::begin(context.items), std::end(context.items), std::back_inserter(items));
                        using CreatedEntities = typename Common::Detail::TupleCat<TCreated, std::tuple<Orm::Table<TEntity, TTableTraits ... >>>::Tuple;
                        Entity<CreatedEntities, TSchemaName, TEntities ... >::Create(items);
                    }

                private:
                    template <typename ... >
                    friend class Entity;

                    using Table = Orm::Detail::Entity<Orm::Table<TEntity, TTableTraits ... >>;
                    using Meta = Reflection::Reflect<TEntity>;

                    static void Create(Context &context)
                    {
                        auto sql = "CREATE TABLE " + context.name + "\n";
                        sql += "(\n";
                        Context itemContext;
                        itemContext.name = context.name;
                        CreateItem<Meta::Fields::Count>(itemContext);

                        using PrimaryKeyFields = typename Orm::Detail::PrimaryKey<Table>::Fields;
                        using PrimaryKey = typename std::conditional
                            <
                                std::tuple_element<0, PrimaryKeyFields>::type::Index::value == Meta::Fields::Count &&
                                    Orm::Detail::HasFieldReference<typename Meta::Fields>::value,
                                PrimaryKeyFields,
                                std::tuple<>
                            >::type;
                        std::string primaryKey;
                        CreatePrimaryKey<PrimaryKey>(itemContext.items, primaryKey);

                        std::copy(std::begin(context.injectedItems), std::end(context.injectedItems),
                                std::back_inserter(itemContext.items));

                        if (!primaryKey.empty())
                            itemContext.items.emplace_back(std::move(primaryKey));

                        sql += boost::algorithm::join(itemContext.items, ",\n");
                        if (!itemContext.items.empty())
                            sql += "\n";
                        sql += ");\n";
                        if (!itemContext.additional.empty())
                        {
                            sql += "\n";
                            sql += boost::algorithm::join(itemContext.additional, "\n");
                            sql += "\n";
                        }
                        context.items.emplace_back(sql);
                    }

                    // Simple types: all int, float pointers, text, datetimes, not reflectable enums
                    template <typename TField>
                    static typename std::enable_if
                        <
                            Serialization::Traits::IsSimple<typename TField::Type>() ||
                                std::is_same<typename TField::Type, boost::posix_time::ptime>::value ||
                                std::is_same<typename TField::Type, boost::posix_time::ptime::date_type>::value ||
                                (std::is_enum<typename TField::Type>::value && !Reflection::IsReflectable<typename TField::Type>()),
                            void
                        >::type
                    CreateItem(Context &context)
                    {
                        Common::Unused(context);
                        using Traits = typename Orm::Detail::FieldTraitsList<Table, TField>::Traits;
                        static constexpr auto IsCounter =
                                Orm::Detail::HasTrait<Orm::Detail::FieldTraits::Trait_Counter, Traits>::value;
                        using FieldType = typename TField::Type;
                        std::string sql = Detail::Indent::Value;
                        sql += Utility::QuoteReserved(Utility::PascalCaseToUnderlining(TField::Name::Value));
                        sql += " ";
                        sql += Type::Simple::TypeName::Get<FieldType, IsCounter>();
                        StringList traits;
                        Detail::FieldTraits::ToStringList<Traits>::Get(traits);
                        if (!traits.empty())
                        {
                            sql += " ";
                            sql += boost::algorithm::join(traits, " ");
                        }
                        context.items.emplace_back(std::move(sql));
                    }

                    // Reflectable enums
                    template <typename TField>
                    static typename std::enable_if
                        <
                            std::is_enum<typename TField::Type>::value && Reflection::IsReflectable<typename TField::Type>(),
                            void
                        >::type
                    CreateItem(Context &context)
                    {
                        Common::Unused(context);
                        using Traits = typename Orm::Detail::FieldTraitsList<Table, TField>::Traits;
                        using FieldType = typename TField::Type;
                        std::string sql = Detail::Indent::Value;
                        sql += Utility::QuoteReserved(Utility::PascalCaseToUnderlining(TField::Name::Value));
                        sql += " ";
                        sql += Utility::EntityName<TSchemaName, typename Reflection::Reflect<FieldType>::Name>::Create();
                        StringList traits;
                        Detail::FieldTraits::ToStringList<Traits>::Get(traits);
                        if (!traits.empty())
                        {
                            sql += " ";
                            sql += boost::algorithm::join(traits, " ");
                        }
                        context.items.emplace_back(std::move(sql));
                    }

                    // Iterable
                    template <typename TField>
                    static typename std::enable_if
                        <
                            Serialization::Traits::IsIterable<typename TField::Type>(),
                            void
                        >::type
                    CreateItem(Context &context)
                    {
                        Common::Unused(context);
                    }

                    // Nested reflectable entity (not a reference)
                    template <typename TField>
                    static typename std::enable_if
                        <
                            Reflection::IsReflectable<typename TField::Type>() && std::is_class<typename TField::Type>::value &&
                                !Orm::Detail::HasTrait
                                    <
                                        Orm::Detail::FieldTraits::Trait_Reference,
                                        typename Orm::Detail::FieldTraitsList<Table, TField>::Traits
                                    >::value,
                            void
                        >::type
                    CreateItem(Context &context)
                    {
                        Context nested;
                        nested.name = context.name;
                        nested.name += "_";
                        nested.name += TField::Name::Value;
                        nested.name = Utility::QuoteReserved(Utility::PascalCaseToUnderlining(nested.name));

                        using FieldType = typename TField::Type;
                        using NestedTable = typename Orm::Table<FieldType>::Create;

                        using OwnerPrimaryKey = typename Orm::Detail::PrimaryKey<Table>::Fields;

                        std::string foreignKey;
                        CreateForeignKey<OwnerPrimaryKey>(context.name, nested.injectedItems, foreignKey);

                        if (!nested.injectedItems.empty())
                            nested.injectedItems.emplace_back(std::move(foreignKey));

                        Entity<TCreated, TSchemaName, NestedTable>::Create(nested);
                        std::copy(std::begin(nested.items), std::end(nested.items), std::back_inserter(context.additional));
                    }

                    // Reference to reflectable entity (not nested)
                    template <typename TField>
                    static typename std::enable_if
                        <
                            Reflection::IsReflectable<typename TField::Type>() && std::is_class<typename TField::Type>::value &&
                                Orm::Detail::HasTrait
                                    <
                                        Orm::Detail::FieldTraits::Trait_Reference,
                                        typename Orm::Detail::FieldTraitsList<Table, TField>::Traits
                                    >::value,
                            void
                        >::type
                    CreateItem(Context &context)
                    {
                        Context nested;
                        nested.name = context.name;
                        nested.name += "_";
                        nested.name += TField::Name::Value;
                        nested.name = Utility::QuoteReserved(Utility::PascalCaseToUnderlining(nested.name));

                        using ReferenceTable = typename Orm::Table<Detail::Reference>
                                ::Field<MIF_FIELD_META(&Detail::Reference::pkReferenceId)>::Counter::NotNull::PrimaryKey
                            ::Create;

                        using FieldType = typename TField::Type;

                        using OwnerPrimaryKey = typename Orm::Detail::PrimaryKey<Table>::Fields;
                        using ReferenceEntitty = Orm::Detail::Entity
                            <
                                typename Orm::Detail::FindEntityByType<FieldType, TCreated>::Entity
                            >;
                        using ReferencePrimaryKey = typename Orm::Detail::PrimaryKey<ReferenceEntitty>::Fields;

                        std::string ownerTableKey;
                        CreateForeignKey<OwnerPrimaryKey>(context.name, nested.injectedItems, ownerTableKey);

                        auto const referenceTableName = Utility::EntityName<TSchemaName, typename Reflection::Reflect<FieldType>::Name>::Create();
                        std::string referenceTableKey;
                        CreateForeignKey<ReferencePrimaryKey>(referenceTableName, nested.injectedItems, referenceTableKey);

                        nested.injectedItems.emplace_back(std::move(ownerTableKey));
                        nested.injectedItems.emplace_back(std::move(referenceTableKey));

                        Entity<TCreated, TSchemaName, ReferenceTable>::Create(nested);
                        std::copy(std::begin(nested.items), std::end(nested.items), std::back_inserter(context.additional));
                    }

                    template <std::size_t I>
                    static typename std::enable_if<I != 0, void>::type
                    CreateItem(Context &context)
                    {
                        using Field = typename Meta::Fields::template Field<Meta::Fields::Count - I>;
                        CreateItem<Field>(context);
                        CreateItem<I - 1>(context);
                    }

                    template <std::size_t I>
                    static typename std::enable_if<I == 0, void>::type
                    CreateItem(Context &context)
                    {
                        Common::Unused(context);
                    }

                    template <typename TKeyFields>
                    static void CreateForeignKey(std::string const &refTable, StringList &items, std::string &key)
                    {
                        StringList fieldsList;
                        CreateForeignKey<TKeyFields, std::tuple_size<TKeyFields>::value>(refTable, items, fieldsList);

                        if (fieldsList.empty())
                            return;

                        key = Detail::Indent::Value;
                        key += "FOREIGN KEY (" + boost::algorithm::join(fieldsList, ", ") + ") ";
                        key += "REFERENCES " + refTable + " ON DELETE CASCADE";
                    }

                    template <typename TKeyFields, std::size_t N>
                    static typename std::enable_if<N != 0, void>::type
                    CreateForeignKey(std::string const &refTable, StringList &items, StringList &fieldsList)
                    {
                        using FieldMeta = typename std::tuple_element<N - 1, TKeyFields>::type;
                        using FieldType = typename FieldMeta::Type;
                        auto const fieldName = Utility::QuoteReserved(Utility::PascalCaseToUnderlining(FieldMeta::Name::Value));
                        std::string fkFieldName = "fk_";
                        fkFieldName += Reflection::Reflect<typename FieldMeta::Class>::Name::Value;
                        fkFieldName += "_";
                        fkFieldName += fieldName;
                        fkFieldName = Utility::QuoteReserved(Utility::PascalCaseToUnderlining(fkFieldName));
                        std::string item = Detail::Indent::Value;
                        item += fkFieldName;
                        item += " ";
                        item += Type::Simple::TypeName::Get<FieldType, false>();
                        item += " ";
                        item += "REFERENCES ";
                        item += refTable;
                        item += "(";
                        item += fieldName;
                        item += ")";
                        item += " ON DELETE CASCADE";
                        fieldsList.emplace_back(fkFieldName);
                        items.emplace_back(std::move(item));
                        CreateForeignKey<TKeyFields, N - 1>(refTable, items, fieldsList);
                    }

                    template <typename TKeyFields, std::size_t N>
                    static typename std::enable_if<N == 0, void>::type
                    CreateForeignKey(std::string const &refTable, StringList &items, StringList &fieldsList)
                    {
                        Common::Unused(refTable);
                        Common::Unused(items);
                        Common::Unused(fieldsList);
                    }

                    template <typename TKeyFields>
                    static void CreatePrimaryKey(StringList &items, std::string &key)
                    {
                        StringList fieldsList;
                        CreatePrimaryKey<TKeyFields, std::tuple_size<TKeyFields>::value>(items, fieldsList);

                        if (fieldsList.empty())
                            return;

                        key = Detail::Indent::Value;
                        key += "PRIMARY KEY (" + boost::algorithm::join(fieldsList, ", ") + ")";
                    }

                    template <typename TKeyFields, std::size_t N>
                    static typename std::enable_if<N != 0, void>::type
                    CreatePrimaryKey(StringList &items, StringList &fieldsList)
                    {
                        using FieldMeta = typename std::tuple_element<N - 1, TKeyFields>::type;
                        using FieldType = typename FieldMeta::Type;
                        auto const fieldName = Utility::QuoteReserved(Utility::PascalCaseToUnderlining(FieldMeta::Name::Value));
                        std::string item = Detail::Indent::Value;
                        item += fieldName;
                        item += " ";
                        item += Type::Simple::TypeName::Get<FieldType, true>();
                        item += " NOT NULL";
                        items.emplace_back(std::move(item));
                        fieldsList.emplace_back(fieldName);
                        CreatePrimaryKey<TKeyFields, N - 1>(items, fieldsList);
                    }

                    template <typename TKeyFields, std::size_t N>
                    static typename std::enable_if<N == 0, void>::type
                    CreatePrimaryKey(StringList &items, StringList &fieldsList)
                    {
                        Common::Unused(items);
                        Common::Unused(fieldsList);
                    }
                };

                template <typename TCreated, typename TSchemaName, typename TEntity, typename ... TEntities>
                class Entity<TCreated, Orm::Detail::Entity<Orm::Schema<TSchemaName, TEntity, TEntities ... >>> final
                {
                public:
                    static void Create(StringList &items)
                    {
                        CreateSchema<TSchemaName>(items);
                        using CreatedEntities = typename Common::Detail::TupleCat
                            <
                                TCreated, std::tuple<Orm::Schema<TSchemaName>>
                            >::Tuple;
                        Entity<CreatedEntities, TSchemaName, TEntity, TEntities ... >::Create(items);
                    }

                private:
                    using SchemaName = typename std::conditional
                        <
                            std::is_same<TSchemaName, Orm::DefaultSchemaName>::value,
                            DefaultSchemaName,
                            TSchemaName
                        >::type;

                    template <typename T>
                    static typename std::enable_if<std::is_same<T, Orm::DefaultSchemaName>::value, void>::type
                    CreateSchema(StringList &items)
                    {
                        Common::Unused(items);
                    }

                    template <typename T>
                    static typename std::enable_if<!std::is_same<T, Orm::DefaultSchemaName>::value, void>::type
                    CreateSchema(StringList &items)
                    {
                        items.emplace_back("CREATE SCHEMA " + Utility::QuoteReserved(Utility::PascalCaseToUnderlining(SchemaName::Value)) + ";\n");
                    }
                };

            }   // namespace Detail
        }   // namespace PostgreSql
    }   // namespace Orm
}   // namespace Mif

#endif  // !MIF_WITH_POSTGRESQL
#endif  // !__MIF_ORM_POSTGRESQL_DETAIL_STRUCTURE_H__
