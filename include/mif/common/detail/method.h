//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_DETAIL_METHOD_H__
#define __MIF_COMMON_DETAIL_METHOD_H__

// STD
#include <tuple>

namespace Mif
{
    namespace Common
    {
        namespace Detail
        {

            template <typename>
            struct Method;

            template <typename TResult, typename TClass, typename ... TParams>
            struct Method<TResult (TClass::*)(TParams ...)>
            {
                using ClassType = TClass;
                using ResultType = TResult;
                using ParamTypeList = std::tuple<TParams ... >;
                enum { IsConst = 0 };
            };

            template <typename TResult, typename TClass, typename ... TParams>
            struct Method<TResult (TClass::*)(TParams ...) const>
            {
                using ClassType = TClass;
                using ResultType = TResult;
                using ParamTypeList = std::tuple<TParams ... >;
                enum { IsConst = 1 };
            };

        }   // namespace Detail
    }   // namespace Common
}   // namespace Mif


#endif  // !__MIF_COMMON_DETAIL_METHOD_H__
