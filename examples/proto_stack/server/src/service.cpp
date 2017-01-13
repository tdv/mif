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

namespace Service
{
    namespace Detail
    {
        namespace
        {

            class Service
                : public IFace
            {
            public:
                Service()
                {
                    LockGuard lock(m_lock);
                    std::cout << "Service" << std::endl;
                }

                ~Service()
                {
                    LockGuard lock(m_lock);
                    std::cout << "~Service" << std::endl;
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
    }   // namespace Detail
}   // namespace Service

MIF_SERVICE_CREATOR
(
    ::Service::Id::Service,
    ::Service::Detail::Service
)
