#ifndef __MIF_COMMON_CREATOR_H__
#define __MIF_COMMON_CREATOR_H__

// STD
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

// MIF
#include "mif/common/index_sequence.h"

namespace Mif
{
    namespace Common
    {

    template <typename TClass, typename ... TBindArgs>
    class Creator final
    {
    public:
        using Type = TClass;
        using TypePtr = std::shared_ptr<Type>;

        Creator(TBindArgs && ... args)
            : m_args{std::forward<TBindArgs>(args) ... }
        {
        }

        template <bool ArgsToHead = true, typename ... TArgs>
        TypePtr Create(TArgs && ... args)
        {
            return CreateObject<ArgsToHead>(
                    reinterpret_cast<MakeIndexSequence<std::tuple_size<TupleType>::value> const *>(0),
                    std::forward<TArgs>(args) ...
                );
        }

    private:
        using TupleType = std::tuple<typename std::decay<TBindArgs>::type ... >;
        TupleType m_args;

        template <bool ArgsToHead, std::size_t ... Indexes, typename ... TArgs>
        typename std::enable_if<ArgsToHead, TypePtr>::type
        CreateObject(IndexSequence<Indexes ... > const *, TArgs && ... args)
        {
            return std::make_shared<Type>(std::forward<TArgs>(args) ... , std::get<Indexes>(m_args) ... );
        }

        template <bool ArgsToHead, std::size_t ... Indexes, typename ... TArgs>
        typename std::enable_if<!ArgsToHead, TypePtr>::type
        CreateObject(IndexSequence<Indexes ... > const *, TArgs && ... args)
        {
            return std::make_shared<Type>(std::get<Indexes>(m_args) ... ,std::forward<TArgs>(args) ... );
        }
    };

    template <typename TClass, typename ... TBindArgs>
    Creator<TClass, TBindArgs ... > MakeCreator(TBindArgs && ... args)
    {
        return Creator<TClass, TBindArgs ... >{std::forward<TBindArgs>(args) ... };
    }

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_CREATOR_H__
