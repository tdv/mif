//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __HELLO_WORLD_SERVER_HELLO_WORLD_SERVICE_H__
#define __HELLO_WORLD_SERVER_HELLO_WORLD_SERVICE_H__

// STD
#include <list>
#include <mutex>

// THIS
#include "common/interface/ihello_world.h"

class HelloWorldService final
    : public IHelloWorld
{
public:
    HelloWorldService();
    ~HelloWorldService();

private:
    using LockType = std::mutex;
    using LockGuard = std::lock_guard<LockType>;

    mutable LockType m_lock;
    std::list<std::string> m_words;

    // IHelloWorld
    virtual void AddWord(std::string const &word) override final;
    virtual std::string GetText() const override final;
    virtual void Clean() override final;
};

#endif  // !__HELLO_WORLD_SERVER_HELLO_WORLD_SERVICE_H__
