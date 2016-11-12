//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_STATIC_STRING_H__
#define __MIF_COMMON_STATIC_STRING_H__

// STD
#include <cstdint>
#include <string>

// MIF
#include "mif/common/index_sequence.h"

#define MIF_DECLARE_SRTING_PROVIDER(name_, value_) \
    struct name_ \
    { \
        static constexpr char const* GetString() \
        { \
            return value_; \
        } \
        static constexpr char GetSize() \
        { \
            return sizeof(value_); \
        } \
    };

namespace Mif
{
    namespace Common
    {

        template <char ... Str>
        struct StaticString
        {
            static std::string const GetString()
            {
                return std::string{ Str ... }.c_str();
            }
        };

        namespace Detail
        {

            template <typename TStringProvider, std::size_t ... Indexes>
            constexpr StaticString<TStringProvider::GetString()[Indexes] ... > MakeStaticString(IndexSequence<Indexes ... > const *);

        }   // namespace Detail

        template <typename TStringProvider>
        using MakeStaticString = decltype(Detail::MakeStaticString<TStringProvider>(
            static_cast<MakeIndexSequence<TStringProvider::GetSize()> const *>(nullptr)));

    }   // namespace Common
}   // namespace Mif


#endif  // !__MIF_COMMON_STATIC_STRING_H__
