//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __PROTO_STACK_COMMON_INTERFACE_IFACE_H__
#define __PROTO_STACK_COMMON_INTERFACE_IFACE_H__

// STD
#include <string>

// MIF
#include <mif/service/iservice.h>

struct IFace
    : public Mif::Service::IService
{
    virtual std::string SayHello(std::string const &text) = 0;
};

#endif  // !__PROTO_STACK_COMMON_INTERFACE_IFACE_H__
