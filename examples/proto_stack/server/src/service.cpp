//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>

// THIS
#include "service.h"

Service::Service()
{
    LockGuard lock(m_lock);
    std::cout << "Service" << std::endl;
}

Service::~Service()
{
    LockGuard lock(m_lock);
    std::cout << "~Service" << std::endl;
}

std::string Service::SayHello(std::string const &text)
{
    std::string result;
    {
        LockGuard lock(m_lock);
        result = "Hello " + text + "!";
        std::cout << "SeyHello: " << result << std::endl;
    }
    return result;
}
