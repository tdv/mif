//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>
#include <mutex>

// MIF
#include <mif/service/creator.h>

// COMMON
#include "common/id/service.h"
#include "common/interface/iface.h"

namespace
{

    class ServiceImpl
        : public IFace
    {
    public:
        ServiceImpl()
        {
            LockGuard lock(m_lock);
            std::cout << "ServiceImpl" << std::endl;
        }

        ~ServiceImpl()
        {
            LockGuard lock(m_lock);
            std::cout << "~ServiceImpl" << std::endl;
        }

    private:
        using LockType = std::mutex;
        using LockGuard = std::lock_guard<LockType>;

        mutable LockType m_lock;

        // IFace
        virtual std::string SayHello(std::string const &text) override final
        {
            std::string result;
            {
                LockGuard lock(m_lock);
                result = "Hello " + text + "!";
                std::cout << "SeyHello: " << result << std::endl;
            }
            return result;
        }

    };

}   // namespace

MIF_SERVICE_CREATOR
(
    ::Service::Id::Service,
    ::ServiceImpl
)
