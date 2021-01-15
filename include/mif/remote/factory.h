//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_FACTORY_H__
#define __MIF_REMOTE_FACTORY_H__

// STD
#include <functional>
#include <memory>

// MIF
#include "mif/net/iconnection.h"
#include "mif/service/ifactory.h"

namespace Mif
{
    namespace Remote
    {

        class Factory
            : public Service::Inherit<Service::IFactory>
        {
        public:
            using ServiceCreator = std::function<Service::IServicePtr (Net::IConnection::ClientPtr, Service::ServiceId)>;

            Factory(std::shared_ptr<Net::IConnection> connection, ServiceCreator const &serviceCreator);

        private:
            std::shared_ptr<Net::IConnection> m_connection;
            ServiceCreator m_serviceCreator;

            // IFactory
            virtual Service::IServicePtr Create(Service::ServiceId id) override final;
        };

    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_FACTORY_H__
