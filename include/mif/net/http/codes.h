//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_CODES_H__
#define __MIF_NET_HTTP_CODES_H__

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            enum class Code
            {
                Ok,
                NoContent,
                MovePerm,
                MoveTemp,
                NotModified,
                BadRequest,
                NotFound,
                BadMethod,
                Unauthorized,
                Internal,
                NotImplemented,
                Unavaliable
            };

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_CODES_H__
