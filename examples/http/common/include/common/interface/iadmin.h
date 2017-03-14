//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __HTTP_COMMON_INTERFACE_IADMIN_H__
#define __HTTP_COMMON_INTERFACE_IADMIN_H__

// STD
#include <string>

// MIF
#include <mif/service/iservice.h>

namespace Service
{

    struct IAdmin
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
        virtual void SetTitle(std::string const &title) = 0;
        virtual void SetBody(std::string const &body) = 0;
        virtual std::string GetPage() const = 0;
    };

}   // namespace Service

#endif  // !__HTTP_COMMON_INTERFACE_IADMIN_H__
