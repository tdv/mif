//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_CRC32_H__
#define __MIF_COMMON_CRC32_H__

// STD
#include <cstdint>
#include <cstring>
#include <string>

namespace Mif
{
    namespace Common
    {
        namespace Detail
        {

        inline constexpr std::uint32_t CalcCrc32K(std::uint32_t c, std::uint32_t k = 0)
        {
            return k < 8 ? CalcCrc32K((c & 1) ? (0xedb88320 ^ (c >> 1)) : (c >> 1), k + 1) : c;
        }

        struct Crc32Table
        {
            std::uint32_t m_data[256];
        };

        template<bool>
        struct SelectCrc32Table
        {
            typedef Crc32Table Type;
        };

        template<>
        struct SelectCrc32Table<false>
        {
        };

        template<typename ... T>
        inline constexpr typename SelectCrc32Table<sizeof...(T) == 256>::Type CalcCrc32Table(std::uint32_t /*n*/, T ... t)
        {
          return Crc32Table {{ t ... }};
        }

        template<typename ... T>
        inline constexpr typename SelectCrc32Table<sizeof...(T) <= 255>::Type CalcCrc32Table(std::uint32_t n, T ... t)
        {
          return CalcCrc32Table(n + 1, t..., CalcCrc32K(n));
        }

        template <typename T>
        struct Crc32TableHolderT
        {
            static constexpr Crc32Table Table = CalcCrc32Table(0);
        };

        template <typename T>
        constexpr Crc32Table Crc32TableHolderT<T>::Table;

        typedef Crc32TableHolderT<void> Crc32TableHolder;

        template<std::uint32_t const I>
        inline constexpr std::uint32_t Crc32(char const *str)
        {
            return (Crc32<I - 1>(str) >> 8) ^ Crc32TableHolder::Table.m_data[(Crc32<I - 1>(str) ^ str[I - 1]) & 0x000000FF];
        }

        template<>
        inline constexpr std::uint32_t Crc32<0>(char const *)
        {
            return 0xFFFFFFFF;
        }

    }   // namespace Detail

    template <std::uint32_t N>
    inline constexpr std::uint32_t Crc32(char const (&str)[N])
    {
        return (Detail::Crc32<N - 1>(str) ^ 0xFFFFFFFF);
    }

    inline std::uint32_t Crc32(void const *data, std::uint32_t bytes)
    {
        std::uint8_t const *buf = reinterpret_cast<std::uint8_t const *>(data);
        std::uint32_t crc = 0;
        crc = crc ^ ~0U;
        while (bytes--)
        {
            crc = Detail::Crc32TableHolder::Table.m_data[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
        }
        return crc ^ ~0U;
    }

    inline std::uint32_t Crc32str(char const *str)
    {
        return Crc32(str, std::strlen(str));
    }

    inline std::uint32_t Crc32str(std::string const &str)
    {
        return Crc32(str.c_str(), str.length());
    }

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_CRC32_H__
