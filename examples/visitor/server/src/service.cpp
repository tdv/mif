//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/common/log.h>
#include <mif/service/creator.h>
#include <mif/service/make.h>

// COMMON
#include "common/id/service.h"
#include "common/interface/iface.h"

namespace Service
{
    namespace Detail
    {
        namespace
        {

            class Message
                : public Mif::Service::Inherit<IMessage>
            {
            public:
                Message(std::string const &message)
                    : m_message{message}
                {
                }

            private:
                std::string m_message;

                // IMessage
                virtual std::string GetText() const override final
                {
                    return m_message;
                }
            };

            class Service
                : public Mif::Service::Inherit<IViewer>
            {
            public:
                Service()
                {
                    MIF_LOG(Info) << "Service";
                }

                ~Service()
                {
                    MIF_LOG(Info) << "~Service";
                }

            private:
                // IViewer
                virtual void Accept(IMessageVisitorPtr visitor) override final
                {
                    if (!visitor)
                        throw std::invalid_argument{"Empty pointer on IMessageVisitor"};

                    MIF_LOG(Info) << "Accept";

                    auto message = Mif::Service::Make<Message, IMessage>("Message from remote service.");

                    visitor->Visit(message);
                }

            };

        }   // namespace
    }   // namespace Detail
}   // namespace Service

MIF_SERVICE_CREATOR
(
    ::Service::Id::Service,
    ::Service::Detail::Service
)
