#ifndef __HELLO_WORLD_COMMON_INTERFACE_IFACE_H__
#define __HELLO_WORLD_COMMON_INTERFACE_IFACE_H__

// STD
#include <string>

// MIF
#include <mif/service/iservice.h>

struct IFace
    : public Mif::Service::IService
{
    virtual std::string SayHello(std::string const &text) = 0;
};

#endif  // !__HELLO_WORLD_COMMON_INTERFACE_IFACE_H__
