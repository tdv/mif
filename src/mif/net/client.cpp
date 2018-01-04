//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// MIF
#include "mif/common/log.h"
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
            if (m_makredAsClosed)
            {
                MIF_LOG(Warning) << "[Mif::Net::Client::OnData] Data will not be processed. Client is closed.";
                return;
            }

            if (buffer.empty())
                throw std::invalid_argument{"[Mif::Net::Client::OnData] No data."};
            ProcessData(std::move(buffer));
        }

        void Client::OnClose()
        {
            bool needToClose = m_makredAsClosed;
            m_makredAsClosed = true;
            if (needToClose)
                Close();
        }

        bool Client::CloseMe()
        {
            if (m_makredAsClosed)
                return false;

            if (auto control = m_control.lock())
            {
                control->CloseMe();
                return true;
            }
            return false;
        }

        bool Client::Post(Common::Buffer buffer)
        {
            if (m_makredAsClosed)
                return false;

            if (auto publisher = m_publisher.lock())
            {
                publisher->Publish(std::move(buffer));
                return true;
            }
            return false;
        }

        bool Client::IsClosed() const
        {
            return m_makredAsClosed;
        }

        void Client::ProcessData(Common::Buffer /*buffer*/)
        {
        }

        void Client::Close()
        {
        }

    }   // namespace Net
}   // namespace Mif
