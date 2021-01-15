//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_ICONTROL_H__
#define __MIF_NET_ICONTROL_H__

namespace Mif
{
    namespace Net
    {

        struct IControl
        {
            virtual ~IControl() = default;
            virtual void CloseMe() = 0;
        };

    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_ICONTROL_H__
