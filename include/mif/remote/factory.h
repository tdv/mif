//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_FACTORY_H__
#define __MIF_REMOTE_FACTORY_H__

// MIF
#include "mif/service/ifactory.h"

namespace Mif
{
    namespace Remote
    {

        class Factory
            : public Service::Inherit<Service::IFactory>
        {
        public:
            Factory();

        private:
            // IFactory
            virtual Service::IServicePtr Create(Service::ServiceId id) override final;
        };

    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_FACTORY_H__
