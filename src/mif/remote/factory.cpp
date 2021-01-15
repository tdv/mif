//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// MIF
#include "mif/remote/factory.h"

namespace Mif
{
    namespace Remote
    {

        Factory::Factory(std::shared_ptr<Net::IConnection> connection, ServiceCreator const &serviceCreator)
            : m_connection{connection}
            , m_serviceCreator{serviceCreator}
        {
            if (!m_connection)
                throw std::invalid_argument{"[Mif::Remote::Factory] Empty connection ptr."};
        }

        Service::IServicePtr Factory::Create(Service::ServiceId id)
        {
            return m_serviceCreator(m_connection->GetClient(), id);
        }

    }   // namespace Remote
}   // namespace Mif
