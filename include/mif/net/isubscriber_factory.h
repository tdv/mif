#ifndef __MIF_NET_ISUBSCRIBER_FACTORY_H__
#define __MIF_NET_ISUBSCRIBER_FACTORY_H__

// STD
#include <memory>

// MIF
#include "mif/net/isubscriber.h"
#include "mif/net/icontrol.h"
#include "mif/net/ipublisher.h"

namespace Mif
{
    namespace Net
    {

        struct ISubscriberFactory
        {
            using ISubscriberPtr = std::shared_ptr<ISubscriber>;
            using IControlPtr = std::weak_ptr<IControl>;
            using IPublisherPtr = std::weak_ptr<IPublisher>;

            virtual ~ISubscriberFactory() = default;
            virtual ISubscriberPtr Create(IControlPtr control, IPublisherPtr publisher) = 0;
        };

    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_ISUBSCRIBER_FACTORY_H__
