//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <utility>

// MIF
#include "mif/common/log.h"

// THIS
#include "http_session.h"

namespace Mif
{
    namespace Net
    {
        namespace Detail
        {

            void HTTPSession::Init(IClientFactory &factory)
            {
                m_client = factory.Create(std::weak_ptr<IControl>(shared_from_this()),
                    std::weak_ptr<IPublisher>(shared_from_this()));
            }

            bool HTTPSession::NeedForClose() const
            {
                return m_needForClose;
            }

            std::exception_ptr HTTPSession::GetException() const
            {
                return m_exception;
            }

            void HTTPSession::Publish(Common::Buffer buffer)
            {
                m_response.swap(buffer);
            }

            void HTTPSession::CloseMe()
            {
                m_needForClose = true;
            }

            Common::Buffer HTTPSession::OnData(Common::Buffer data)
            {
                try
                {
                    if (!m_response.empty())
                        Common::Buffer{}.swap(m_response);
                    if (m_exception)
                        std::exception_ptr{}.swap(m_exception);

                    m_client->OnData(std::move(data));

                    m_timestamp = std::time(nullptr);
                }
                catch (std::exception const &e)
                {
                    m_exception = std::current_exception();
                    CloseMe();
                    MIF_LOG(Warning) << "[Mif::Net::Detail::HTTPSession::OnData]. "
                        << "Failed to process data. Error: " << e.what();
                }

                return std::move(m_response);
            }

        }   // namespace Detail
    }   // namespace Net
}   // namespace Mif
