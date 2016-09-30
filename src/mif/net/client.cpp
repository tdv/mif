// STD
#include <stdexcept>

// MIF
#include "mif/net/client.h"

namespace Mif
{
    namespace Net
    {

        Client::Client(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher)
            : m_control{control}
            , m_publisher{publisher}
        {
        }

        void Client::OnData(Common::Buffer buffer)
        {
            if (buffer.empty())
                throw std::invalid_argument{"[Mif::Net::Client::OnData] No data."};
            ProcessData(std::move(buffer));
        }

        bool Client::CloseMe()
        {
            if (auto control = m_control.lock())
            {
                control->CloseMe();
                return true;
            }
            return false;
        }

        bool Client::Post(Common::Buffer buffer)
        {
            if (auto publisher = m_publisher.lock())
            {
                publisher->Publish(std::move(buffer));
                return true;
            }
            return false;
        }

        void Client::ProcessData(Common::Buffer /*buffer*/)
        {
        }

    }   // namespace Net
}   // namespace Mif
