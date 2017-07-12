//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include "mif/remote/factory.h"

namespace Mif
{
    namespace Remote
    {

        Factory::Factory()
        {
        }

        Service::IServicePtr Factory::Create(Service::ServiceId id)
        {
            (void)id;
            return {};
        }

    }   // namespace Remote
}   // namespace Mif
