//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_IINPUT_PACK_H__
#define __MIF_NET_HTTP_IINPUT_PACK_H__

// STD
#include <cstdint>
#include <map>
#include <string>

// MIF
#include "mif/common/types.h"
#include "mif/net/http/codes.h"
#include "mif/net/http/methods.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {

            struct IInputPack
            {
                using Params = std::map<std::string, std::string>;
                using Headers = std::map<std::string, std::string>;

                virtual ~IInputPack() = default;

                virtual Method::Type GetType() const = 0;

                virtual Code GetCode() const = 0;
                virtual std::string GetReason() const = 0;

                virtual std::string GetHost() const = 0;
                virtual std::uint16_t GetPort() const = 0;

                virtual std::string GetSchema() const = 0;
                virtual std::string GetUserInfo() const = 0;
                virtual std::string GetPath() const = 0;
                virtual std::string GetQuery() const = 0;
                virtual std::string GetFragment() const = 0;

                virtual Params GetParams() const = 0;

                virtual Headers GetHeaders() const = 0;

                virtual Common::Buffer GetData() const = 0;
            };

        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_IINPUT_PACK_H__
