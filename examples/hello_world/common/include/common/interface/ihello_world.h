//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __HELLO_WORLD_COMMON_INTERFACE_IHELLO_WORLD_H__
#define __HELLO_WORLD_COMMON_INTERFACE_IHELLO_WORLD_H__

// STD
#include <string>

// MIF
#include <mif/service/iservice.h>

namespace Service
{

    struct IHelloWorld
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
        virtual void AddWord(std::string const &word) = 0;
        virtual std::string GetText() const = 0;
        virtual void Clean() = 0;
    };

}   // namespace Service

#endif  // !__HELLO_WORLD_COMMON_INTERFACE_IHELLO_WORLD_H__
