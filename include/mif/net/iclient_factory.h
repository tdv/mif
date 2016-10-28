//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_ICLIENT_FACTORY_H__
#define __MIF_NET_ICLIENT_FACTORY_H__

// STD
#include <memory>

// MIF
#include "mif/net/client.h"

namespace Mif
{
    namespace Net
    {

        struct IClientFactory
        {
            using ClientPtr = std::shared_ptr<Client>;
            using IControlPtr = std::weak_ptr<IControl>;
            using IPublisherPtr = std::weak_ptr<IPublisher>;

            virtual ~IClientFactory() = default;
            virtual ClientPtr Create(IControlPtr control, IPublisherPtr publisher) = 0;
        };

    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_ICLIENT_FACTORY_H__
