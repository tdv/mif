//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_SERIALIZATION_DETAIL_TAG_H__
#define __MIF_REMOTE_SERIALIZATION_DETAIL_TAG_H__

// MIF
#include "mif/common/static_string.h"

namespace Mif
{
    namespace Remote
    {
        namespace Serialization
        {
            namespace Detail
            {
                namespace Tag
                {

                    using Pack = MIF_STATIC_STR("package");
                    using Uuid = MIF_STATIC_STR("uuid");
                    using Type = MIF_STATIC_STR("type");
                    using Request = MIF_STATIC_STR("request");
                    using Response = MIF_STATIC_STR("response");
                    using Instsnce = MIF_STATIC_STR("instance");
                    using Interface = MIF_STATIC_STR("interface");
                    using Method = MIF_STATIC_STR("method");
                    using Param = MIF_STATIC_STR("prm");
                    using Exception = MIF_STATIC_STR("exception");

                }   // namespace Tag
            }   // namespace Detail
        }   // namespace Serialization
    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_SERIALIZATION_DETAIL_TAG_H__
