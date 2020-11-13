//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_MAKE_H__
#define __MIF_SERVICE_MAKE_H__

// STD
#include <utility>

// MIF
#include "mif/service/iservice.h"
#include "mif/service/detail/service.h"

namespace Mif
{
    namespace Service
    {

        template <typename TImpl, typename TRet, typename ... TArgs>
        inline TServicePtr<TRet> Make(TArgs && ... args)
        {
            TServicePtr<Detail::Fake_Service__> service{new Detail::Service_Impl__<TImpl>(std::forward<TArgs>(args) ... )};
            return Cast<TRet>(service);
        }

        template <typename TImpl, typename ... TArgs>
        inline IServicePtr Make(TArgs && ... args)
        {
            return Make<TImpl, IService, TArgs ... >(std::forward<TArgs>(args) ... );
        }

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_MAKE_H__
