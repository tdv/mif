//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_CLIENT_H__
#define __MIF_NET_CLIENT_H__

// STD
#include <atomic>
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

            bool IsClosed() const;

            // IHandler
            virtual void OnData(Common::Buffer buffer) override final;
            virtual void OnClose() override final;

        private:
            std::atomic<bool> m_makredAsClosed{false};
            std::weak_ptr<IControl> m_control;
            std::weak_ptr<IPublisher> m_publisher;

        protected:
            bool CloseMe();
            bool Post(Common::Buffer buffer);

            virtual void ProcessData(Common::Buffer /*buffer*/);
            virtual void Close();
        };

    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_CLIENT_H__
