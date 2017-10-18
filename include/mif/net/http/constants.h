//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_CONSTANTS_H__
#define __MIF_NET_HTTP_CONSTANTS_H__

// MIF
#include "mif/common/static_string.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Constants
            {
                namespace Header
                {

                    using Connection = MIF_STATIC_STR("Connection");
                    using ContentType = MIF_STATIC_STR("Content-Type");
                    using Session = MIF_STATIC_STR("X-Mif-Session");

                }   // namespace Header

                namespace Value
                {
                    namespace Connection
                    {

                        using KeepAlive = MIF_STATIC_STR("keep-alive");
                        using Close = MIF_STATIC_STR("close");

                    }   // namespace Connection
                }   // namespace Value
            }   // namespace Constants
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_CONSTANTS_H__
