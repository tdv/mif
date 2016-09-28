#ifndef __MIF_NET_CLIENT_H__
#define __MIF_NET_CLIENT_H__

// STD
#include <memory>

// MIF
#include "mif/net/ihandler.h"
#include "mif/net/icontrol.h"
#include "mif/net/ipublisher.h"

namespace Mif
{
    namespace Net
    {

        class Client
            : public std::enable_shared_from_this<Client>
            , public IHandler
        {
        public:
            Client(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher);

            // IHandler
            virtual void OnData(Common::Buffer buffer) override final;

        private:
            std::weak_ptr<IControl> m_control;
            std::weak_ptr<IPublisher> m_publisher;

        protected:
            bool CloseMe();
            bool Post(Common::Buffer &&buffer);

            virtual void ProcessData(Common::Buffer /*buffer*/);
        };

    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_CLIENT_H__
