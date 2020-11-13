//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     04.2018
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_PARAM_PACK_H__
#define __MIF_NET_HTTP_DETAIL_PARAM_PACK_H__

// STD
#include <cstdint>
#include <map>
#include <string>

// MIF
#include "mif/common/index_sequence.h"
#include "mif/common/unused.h"
#include "mif/reflection/reflection.h"
#include "mif/serialization/traits.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                template <typename T, typename TConverter>
                class ParamPack final
                {
                public:
                    using ValueType = typename std::decay<T>::type;
                    using ParamsType = std::map<std::string, std::string>;

                    ParamPack(ParamsType const &params)
                    {
                        GetParams(params, static_cast<Common::MakeIndexSequence<Fields::Count> const *>(0));
                    }

                    ValueType const& Get() const
                    {
                        return m_pack;
                    }

                private:
                    using Meta = Reflection::Reflect<ValueType>;
                    using Fields = typename Meta::Fields;

                    static_assert(Reflection::IsReflectable<ValueType>() && std::is_class<ValueType>::value,
                            "[Mif::Net::Http::Detail::ParamPack] Type must be reflectable struct.");

                    ValueType m_pack;

                    template <std::size_t ... Indexes>
                    void GetParams(ParamsType const &params, Common::IndexSequence<Indexes ... > const *)
                    {
                        Common::Unused((m_pack.*Fields::template Field<Indexes>::Access() = GetParam<Indexes>(params)) ... );
                    }

                    template <std::size_t Index>
                    typename std::enable_if
                        <
                            !Serialization::Traits::IsOptional<typename Fields::template Field<Index>::Type>(),
                            typename Fields::template Field<Index>::Type
                        >::type
                    GetParam(ParamsType const &params) const
                    {
                        using Field = typename Fields::template Field<Index>;
                        using FieldType = typename Field::Type;

                        std::string const name = Field::Name::Value;

                        auto const iter = params.find(name);
                        if (iter == std::end(params))
                            throw std::runtime_error{"[Mif::Net::Http::Detail::ParamPack::GetParam] \"" + name + "\" not found."};

                        return TConverter::template Convert<FieldType>(iter->second);
                    }

                    template <std::size_t Index>
                    typename std::enable_if
                        <
                            Serialization::Traits::IsOptional<typename Fields::template Field<Index>::Type>(),
                            typename Fields::template Field<Index>::Type
                        >::type
                    GetParam(ParamsType const &params) const
                    {
                        using Field = typename Fields::template Field<Index>;
                        using FieldType = typename std::decay<typename Field::Type::value_type>::type;

                        std::string const name = Field::Name::Value;

                        auto const iter = params.find(name);
                        if (iter == std::end(params))
                            return {};

                        return TConverter::template Convert<FieldType>(iter->second);
                    }
                };

                template <typename T, typename TConverter>
                std::true_type IsParamPack(ParamPack<T, TConverter> const *);
                std::false_type IsParamPack(...);

                template <typename T>
                inline constexpr bool IsParamPack()
                {
                    return decltype(IsParamPack(static_cast<typename std::decay<T>::type const *>(0)))::value;
                }

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_PARAM_PACK_H__
