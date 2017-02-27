//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_CONSTANTS_H__
#define __MIF_NET_HTTP_DETAIL_CONSTANTS_H__

// MIF
#include "mif/common/static_string.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {
                namespace Constants
                {
                    namespace Header
                    {

                        MIF_DECLARE_SRTING_PROVIDER(Connection, "Connection")
                        MIF_DECLARE_SRTING_PROVIDER(Session, "X-Mif-Session")

                    }   // namespace Header

                    namespace Value
                    {
                        namespace Connection
                        {

                            MIF_DECLARE_SRTING_PROVIDER(KeepAlive, "keep-alive")
                            MIF_DECLARE_SRTING_PROVIDER(Close, "close")

                        }   // namespace Connection
                    }   // namespace Value
                }   // namespace Constants
            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_CONSTANTS_H__
