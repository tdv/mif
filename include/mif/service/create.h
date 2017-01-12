//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_CREATE_H__
#define __MIF_SERVICE_CREATE_H__

// STD
#include <utility>

// MIF
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Service
    {
        namespace Detail
        {
            namespace Creators
            {

                template <ServiceId Id, typename ... TArgs>
                IServicePtr Create(TArgs ... args);

            }   // namespace Creators
        }   // namespace Detail

        template <ServiceId Id, typename T, typename ... TArgs>
        TServicePtr<T> Create(TArgs && ... args)
        {
            return Cast<T>(Detail::Creators::Create<Id>(std::forward<TArgs>(args) ... ));
        }

        template <ServiceId Id, typename ... TArgs>
        IServicePtr Create(TArgs && ... args)
        {
            return Create<Id, IService, TArgs ... >(std::forward<TArgs>(args) ... );
        }

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_CREATE_H__
