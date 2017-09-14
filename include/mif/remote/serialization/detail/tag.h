//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2017 tdv
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

                    MIF_DECLARE_SRTING_PROVIDER(Pack, "package")
                    MIF_DECLARE_SRTING_PROVIDER(Uuid, "uuid")
                    MIF_DECLARE_SRTING_PROVIDER(Type, "type")
                    MIF_DECLARE_SRTING_PROVIDER(Request, "request")
                    MIF_DECLARE_SRTING_PROVIDER(Response, "response")
                    MIF_DECLARE_SRTING_PROVIDER(Instsnce, "instance")
                    MIF_DECLARE_SRTING_PROVIDER(Interface, "interface")
                    MIF_DECLARE_SRTING_PROVIDER(Method, "method")
                    MIF_DECLARE_SRTING_PROVIDER(Param, "prm")
                    MIF_DECLARE_SRTING_PROVIDER(Exception, "exception")

                }   // namespace Tag
            }   // namespace Detail
        }   // namespace Serialization
    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_SERIALIZATION_DETAIL_TAG_H__
