#ifndef __HELLO_WORLD_SERVER_SERVICE_H__
#define __HELLO_WORLD_SERVER_SERVICE_H__

// STD
#include <mutex>

// THIS
#include "common/interface/iface.h"

class Service final
    : public IFace
{
public:
    Service();
    ~Service();

private:
    using LockType = std::mutex;
    using LockGuard = std::lock_guard<LockType>;

    mutable LockType m_lock;

    // IFace
    virtual std::string SayHello(std::string const &text) override final;
};

#endif  // !__HELLO_WORLD_SERVER_SERVICE_H__
