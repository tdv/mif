#ifndef __MIF_NET_CLIENT_H__
#define __MIF_NET_CLIENT_H__

// STD
#include <memory>
#include <exception>

// MIF
#include "mif/net/isubscriber.h"
#include "mif/net/icontrol.h"
#include "mif/net/ipublisher.h"

namespace Mif
{
    namespace Net
    {

        class Client
            : public std::enable_shared_from_this<Client>
            , public ISubscriber
        {
        public:
            Client(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher)
                : m_control{control}
                , m_publisher{publisher}
            {
            }

        private:
            std::weak_ptr<IControl> m_control;
            std::weak_ptr<IPublisher> m_publisher;

            // ISubscriber
            virtual void OnData(Common::Buffer buffer) override final
            {
                if (!buffer.first || !buffer.second)
                    throw std::invalid_argument{"[Mif::Net::Client::OnData] No data."};
                ProcessData(std::move(buffer));
            }

        protected:
            bool CloseMe()
            {
                if (auto control = m_control.lock())
                {
                    control->CloseMe();
                    return true;
                }
                return false;
            }

            bool Post(Common::Buffer &&buffer)
            {
                if (auto publisher = m_publisher.lock())
                {
                    publisher->Publish(std::move(buffer));
                    return true;
                }
                return false;
            }

            virtual void ProcessData(Common::Buffer /*buffer*/)
            {
            }
        };

    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_CLIENT_H__
