//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/tcp_service_client.h>
#include <mif/common/log.h>

// COMMON
#include "common/id/service.h"
#include "common/ps/iface.h"

namespace Service
{
    namespace Detail
    {
        namespace
        {

            class MessageVisitor
                : public Mif::Service::Inherit<IMessageVisitor>
            {
            private:
                // IMessageVisitor
                virtual void Visit(IMessagePtr message) override final
                {
                    if (!message)
                        throw std::invalid_argument{"Empty message pointer."};

                    MIF_LOG(Info) << "Message: " << message->GetText();
                }
            };

        }   // namespace
    }   // namespace Detail
}   // namespace Service

class Application
    : public Mif::Application::TcpServiceClient
{
public:
    using TcpServiceClient::TcpServiceClient;

private:
    // Mif.Application.TcpServiceClient
    virtual void Init(Mif::Service::IFactoryPtr factory) override final
    {
        auto service = factory->Create<Service::IViewer>(Service::Id::Service);

        auto visitor = Mif::Service::Make<Service::Detail::MessageVisitor, Service::IMessageVisitor>();
        service->Accept(visitor);
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
