//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_IOUTPUT_PACK_H__
#define __MIF_NET_HTTP_IOUTPUT_PACK_H__

// STD
#include <string>

// MIF
#include "mif/common/types.h"
#include "mif/net/http/codes.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            struct IOutputPack
            {
                virtual ~IOutputPack() = default;

                virtual Code GetCode() const = 0;

                virtual void SetCode(Code code) = 0;
                virtual void SetReason(std::string const &reason) = 0;

                virtual void SetHeader(std::string const &key, std::string const &value) = 0;
                virtual void SetData(Common::Buffer buffer) = 0;
                virtual void SetData(Common::BufferPtr buffer) = 0;
            };

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_IOUTPUT_PACK_H__
