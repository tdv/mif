#ifndef __HELLO_WORLD_COMMON_INTERFACE_IHELLO_WORLD_H__
#define __HELLO_WORLD_COMMON_INTERFACE_IHELLO_WORLD_H__

// STD
#include <string>

// MIF
#include <mif/service/iservice.h>

struct IHelloWorld
    : public Mif::Service::IService
{
    virtual void AddWord(std::string const &word) = 0;
    virtual std::string GetText() const = 0;
    virtual void Clean() = 0;
};

#endif  // !__HELLO_WORLD_COMMON_INTERFACE_IHELLO_WORLD_H__
