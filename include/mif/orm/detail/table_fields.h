//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_DETAIL_TABLE_FIELDS_H__
#define __MIF_ORM_DETAIL_TABLE_FIELDS_H__

// STD
#include <string>
#include <tuple>
#include <type_traits>

// BOOST
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/preprocessor.hpp>
#include <boost/vmd/is_empty.hpp>
#include <boost/vmd/to_seq.hpp>

// MIF
#include "mif/common/crc32.h"
#include "mif/common/detail/tuple_utility.h"
#include "mif/reflection/reflection.h"
#include "mif/serialization/traits.h"

namespace Mif
{
    namespace Orm
    {
        namespace Detail
        {
            template <typename, typename, typename, typename, typename, typename>
            class FieldInfo;

            namespace FieldTraits
            {

                template <std::size_t>
                struct FieldTrait;

#define MIF_ORM_FIELD_NO_INCOMPATIBLE_TRAITS_IMPL(...)

#define MIF_ORM_FIELD_INCOMPATIBLE_TRAITS_IMPL(r_, _, I, incompatible_trait_) \
    BOOST_PP_COMMA_IF(I) FieldTrait<Common::Crc32(BOOST_PP_STRINGIZE(incompatible_trait_))>

#define MIF_ORM_FIELD_TRAIT_BEGIN_IMPL(trait_, is_unique_, ... ) \
    enum \
    { \
        Trait_ ## trait_ = Common::Crc32(#trait_) \
    }; \
    template <> \
    class FieldTrait<Trait_ ## trait_> \
    { \
    private: \
        template <typename> \
        class trait_ ## _Holder; \
        template \
        < \
            typename TTableEntity, \
            typename TFieldMeta, \
            typename TTraits, \
            typename TAvailableTraits, \
            typename TUniqueTraits, \
            typename TDeclaredFields \
        > \
        class trait_ ## _Holder <FieldInfo<TTableEntity, TFieldMeta, TTraits, TAvailableTraits, TUniqueTraits, TDeclaredFields>> \
        { \
        private: \
            using CurrentTrait = std::tuple<FieldTrait<Trait_ ## trait_>>; \
            using NewFieldTraits = Common::Detail::MakeUniqueTuple \
                < \
                    typename Common::Detail::TupleCat<TTraits, CurrentTrait>::Tuple \
                >; \
            using IncompatibleTraits = std::tuple \
                < \
                    BOOST_PP_SEQ_FOR_EACH_I \
                    ( \
                        BOOST_PP_IF \
                        ( \
                            BOOST_VMD_IS_EMPTY(__VA_ARGS__), \
                            MIF_ORM_FIELD_NO_INCOMPATIBLE_TRAITS_IMPL, \
                            MIF_ORM_FIELD_INCOMPATIBLE_TRAITS_IMPL \
                        ), \
                        _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) \
                    ) \
                >; \
            using UniqueTraits = Common::Detail::MakeUniqueTuple \
                < \
                    typename Common::Detail::TupleCat \
                        < \
                            TUniqueTraits, \
                            typename std::conditional<is_unique_, CurrentTrait, std::tuple<>>::type \
                        >::Tuple \
                >; \
            using AllIncompatibleTraits = Common::Detail::MakeUniqueTuple \
                < \
                    typename Common::Detail::TupleCat<UniqueTraits, IncompatibleTraits>::Tuple \
                >; \
            using NewAvailableTraits = typename Common::Detail::TupleDifference \
                < \
                    TAvailableTraits, \
                    Common::Detail::MakeUniqueTuple \
                        < \
                            typename Common::Detail::TupleCat<CurrentTrait, AllIncompatibleTraits>::Tuple \
                        > \
                >; \
            using FieldNextTraits = FieldInfo<TTableEntity, TFieldMeta, NewFieldTraits, NewAvailableTraits, UniqueTraits, TDeclaredFields> ; \
        public: \
            using trait_ = FieldNextTraits; \
        }; \
    public: \
        template <typename TField> \
        using Type = trait_ ## _Holder <TField>;

#define MIF_ORM_FIELD_TRAIT_END_IMPL() \
    };

#define MIF_ORM_FIELD_TRAIT_IMPL(trait_, is_unique_, ... ) \
    MIF_ORM_FIELD_TRAIT_BEGIN_IMPL(trait_, is_unique_, __VA_ARGS__) \
    MIF_ORM_FIELD_TRAIT_END_IMPL()

                MIF_ORM_FIELD_TRAIT_IMPL(Counter, false, Nullable)
                MIF_ORM_FIELD_TRAIT_IMPL(NotNull, false, Nullable)
                MIF_ORM_FIELD_TRAIT_IMPL(Nullable, false, NotNull, PrimaryKey, Unique)
                MIF_ORM_FIELD_TRAIT_IMPL(PrimaryKey, true, Nullable)
                MIF_ORM_FIELD_TRAIT_IMPL(Unique, false, Nullable)
                MIF_ORM_FIELD_TRAIT_IMPL(WithTimezone, false, WithoutTimezone)
                MIF_ORM_FIELD_TRAIT_IMPL(WithoutTimezone, false, WithTimezone)
                MIF_ORM_FIELD_TRAIT_IMPL(Reference, false)

#undef MIF_ORM_FIELD_NO_INCOMPATIBLE_TRAITS_IMPL
#undef MIF_ORM_FIELD_INCOMPATIBLE_TRAITS_IMPL
#undef MIF_ORM_FIELD_TRAIT_BEGIN_IMPL
#undef MIF_ORM_FIELD_TRAIT_END_IMPL
#undef MIF_ORM_FIELD_TRAIT_IMPL

                using SimpleTypeTraits = Common::Detail::MakeUniqueTuple
                        <
                            std::tuple
                            <
                                FieldTrait<Trait_NotNull>,
                                FieldTrait<Trait_Nullable>,
                                FieldTrait<Trait_PrimaryKey>,
                                FieldTrait<Trait_Unique>
                            >
                        >;

                using CounterTypeTraits = Common::Detail::MakeUniqueTuple
                        <
                            typename Common::Detail::TupleCat
                            <
                                SimpleTypeTraits,
                                std::tuple<FieldTrait<Trait_Counter>>
                            >::Tuple
                        >;

                using DateTypeTraits = Common::Detail::MakeUniqueTuple
                        <
                            typename Common::Detail::TupleCat
                            <
                                SimpleTypeTraits,
                                std::tuple<FieldTrait<Trait_WithTimezone>>,
                                std::tuple<FieldTrait<Trait_WithoutTimezone>>
                            >::Tuple
                        >;

                using ComplexTypeTraits = std::tuple
                        <
                            FieldTrait<Trait_Reference>
                        >;

                using NoTypeTraits = std::tuple<>;

                using AllTypeTraits = std::tuple
                    <
                        FieldTrait<Trait_Counter>,
                        FieldTrait<Trait_WithTimezone>,
                        FieldTrait<Trait_WithoutTimezone>,
                        FieldTrait<Trait_NotNull>,
                        FieldTrait<Trait_Nullable>,
                        FieldTrait<Trait_Unique>,
                        FieldTrait<Trait_PrimaryKey>,
                        FieldTrait<Trait_Reference>
                    >;

                template <typename T>
                using AvailableTraits = typename std::conditional
                    <
                        std::is_floating_point<T>::value || std::is_same<T, std::string>::value || std::is_enum<T>::value,
                        SimpleTypeTraits,
                        typename std::conditional
                        <
                            std::is_integral<T>::value,
                            CounterTypeTraits,
                            typename std::conditional
                            <
                                std::is_same<T, boost::posix_time::ptime>::value,
                                DateTypeTraits,
                                typename std::conditional
                                <
                                    Reflection::IsReflectable<T>() && std::is_class<T>::value,
                                    ComplexTypeTraits,
                                    NoTypeTraits
                                >::type
                            >::type
                        >::type
                    >::type;

            }   // namespace FieldTraits

            template <typename, typename>
            struct InheritFieldTraits;

            template <typename TField, typename TTrait, typename ... TTraits>
            struct InheritFieldTraits<TField, std::tuple<TTrait, TTraits ... >>
                : public TTrait::template Type<TField>
                , public InheritFieldTraits<TField, std::tuple<TTraits ... >>
            {
            };

            template <typename TField>
            struct InheritFieldTraits<TField, std::tuple<>>
            {
            };

            template <typename TFieldMeta, typename TTraits>
            struct FieldDescr final
            {
                using FieldMeta = TFieldMeta;
                using Traits = TTraits;
            };

            template
            <
                typename TTableEntity,
                typename TFieldMeta,
                typename TTraits,
                typename TAvailableTraits,
                typename TUniqueTraits,
                typename TDeclaredFields
            >
            class FieldInfo
                : public InheritFieldTraits
                    <
                        FieldInfo<TTableEntity, TFieldMeta, TTraits, TAvailableTraits, TUniqueTraits, TDeclaredFields>,
                        TAvailableTraits
                    >
            {
            private:
                using DeclaredFields = typename Common::Detail::TupleCat<TDeclaredFields, std::tuple<FieldDescr<TFieldMeta, TTraits>>>::Tuple;

            public:
                using Create = typename TTableEntity::template CreateTable<DeclaredFields>;

                template <typename TField>
                using Field = FieldInfo<TTableEntity, TField, std::tuple<>,
                        FieldTraits::AvailableTraits<typename TField::Type>, TUniqueTraits,
                        DeclaredFields>;

            private:
                static_assert(std::is_same<typename TTableEntity::EntityType, typename TFieldMeta::Class>::value,
                        "[Mif::Orm::Detail::FieldInfo] You cann't use a field from another struct.");
                static_assert(!Serialization::Traits::IsSmartPointer<typename TFieldMeta::Type>(),
                        "[Mif::Orm::Detail::FieldInfo] Fields with smart pointer are not supported.");

                template <typename, typename>
                struct AlreadyDeclared;

                template <typename T, typename ... TDecl>
                struct AlreadyDeclared<T, std::tuple<TDecl ... >>
                    : public Common::Detail::Disjunction<std::is_same<typename T::Index, typename TDecl::FieldMeta::Index> ... >
                {
                };

                static_assert(!AlreadyDeclared<TFieldMeta, TDeclaredFields>::value,
                        "[Mif::Orm::Detail::FieldInfo] "
                        "You can not override the properties of fields that have already been declared."
                    );
            };

        }   // namespace Detail
    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_DETAIL_TABLE_FIELDS_H__
