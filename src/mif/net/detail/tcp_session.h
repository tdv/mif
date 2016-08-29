#ifndef __MIF_NET_DETAIL_TCP_SESSION_H__
#define __MIF_NET_DETAIL_TCP_SESSION_H__

// STD
#include <memory>

// BOOST
#include <boost/asio/ip/tcp.hpp>

// MIF
#include "mif/common/thread_pool.h"
#include "mif/net/isubscriber_factory.h"

namespace Mif
{
    namespace Net
    {
        namespace Detail
        {

            class TCPSession final
                : public std::enable_shared_from_this<TCPSession>
                , public IPublisher
                , public IControl
            {
            public:
                TCPSession(boost::asio::ip::tcp::socket socket,
                    Common::ThreadPool &workers, ISubscriberFactory &factory);

                void Start();

            private:
                boost::asio::ip::tcp::socket m_socket;
                Common::ThreadPool &m_workers;
                ISubscriberFactory &m_factory;
                std::shared_ptr<ISubscriber> m_subscriber;

                //----------------------------------------------------------------------------
                // IPublisher
                virtual void Publish(Common::Buffer buffer) override;

                //----------------------------------------------------------------------------
                // IControl
                virtual void CloseMe() override;

                //----------------------------------------------------------------------------
                void DoRead();
            };

        }   // namespace Detail
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_DETAIL_TCP_SESSION_H__
