#ifndef __MIF_REFLECTION_REFLECTION_H__
#define __MIF_REFLECTION_REFLECTION_H__

// STD
#include <cstdint>

namespace Mif
{
    namespace Reflection
    {
        namespace Detail
        {

            struct Base
            {
            };

            struct Field
            {
                using Name = void;
                static constexpr std::size_t Id = 0;
                using Type = void;
            };

            template <typename T>
            struct FieldsList
            {
                static constexpr std::size_t Count = {0};
            };

            template <typename T>
            struct Class
            {
                using Namespace = void;
                using Name = typename Registry::Registry<T>::Type::TypeNameProvider;
                using Base = void;
                using Type = void;
                using Fields = FieldsList<T>;
            };

        }   // namespace Detail

        template <typename T>
        using Reflect = Detail::Class<T>;

    }   // namespace Reflection
}   // namespace Mif

#endif  // !__MIF_REFLECTION_REFLECTION_H__
