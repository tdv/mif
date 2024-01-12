//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __INHERITANCE_COMMON_INTERFACE_IFACE_H__
#define __INHERITANCE_COMMON_INTERFACE_IFACE_H__

// STD
#include <cstdint>
#include <string>

// MIF
#include <mif/service/iservice.h>

namespace Service
{

    struct IHuman
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
        virtual std::string GetName() const = 0;
        virtual std::size_t GetAge() const = 0;
    };

    struct IDeveloper
        : public Mif::Service::Inherit<IHuman>
    {
        virtual void Development() = 0;
        virtual void BugFixes() = 0;
    };

    struct IManager
        : public Mif::Service::Inherit<IHuman>
    {
        virtual void Planning() = 0;
        virtual void Monitoring() = 0;
    };

    struct IEmployee
        : public Mif::Service::Inherit<IDeveloper, IManager>
    {
        virtual double GetRate() = 0;
    };

}   // namespace Service

#endif  // !__INHERITANCE_COMMON_INTERFACE_IFACE_H__
