//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_METHODS_H__
#define __MIF_NET_HTTP_METHODS_H__

// STD
#include <set>

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Method
            {

                enum class Type
                {
                    Get,
                    Post,
                    Head,
                    Put,
                    Delete,
                    Options,
                    Trqce,
                    Connect,
                    Patch
                };

            }   // namespace Method

            using Methods = std::set<Method::Type>;

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_METHODS_H__
