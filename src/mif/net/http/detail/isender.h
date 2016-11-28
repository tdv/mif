//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_ISENDER_H__
#define __MIF_NET_HTTP_DETAIL_ISENDER_H__

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                struct ISender
                {
                    virtual ~ISender() = default;
                    virtual void Send() = 0;
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_ISENDER_H__
