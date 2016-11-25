//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_IRESPONSE_H__
#define __MIF_NET_HTTP_IRESPONSE_H__

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            struct IResponse
            {
                virtual ~IResponse() = default;
            };

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_IRESPONSE_H__
