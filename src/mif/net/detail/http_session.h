//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_DETAIL_HTTP_SESSION_H__
#define __MIF_NET_DETAIL_HTTP_SESSION_H__

// STD
#include <ctime>
#include <memory>
#include <stdexcept>

// MIF
#include "mif/net/iclient_factory.h"

namespace Mif
{
    namespace Net
    {
        namespace Detail
        {

            class HTTPSession final
                : public std::enable_shared_from_this<HTTPSession>
                , public IPublisher
                , public IControl
            {
            public:
                void Init(IClientFactory &factory);
                bool NeedForClose() const;
                std::exception_ptr GetException() const;
                Common::Buffer OnData(Common::Buffer data);

            private:
                std::time_t m_timestamp{std::time(nullptr)};
                bool m_needForClose{false};
                Common::Buffer m_response;
                std::exception_ptr m_exception{};
                IClientFactory::ClientPtr m_client;

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

#endif  // !__MIF_NET_DETAIL_HTTP_SESSION_H__
