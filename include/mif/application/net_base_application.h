//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_APPLICATION_NET_BASE_APPLICATION_H__
#define __MIF_APPLICATION_NET_BASE_APPLICATION_H__

// STD
#include <chrono>
#include <cstdint>
#include <string>

// MIF
#include "mif/application/application.h"

namespace Mif
{
    namespace Application
    {

        class NetBaseApplication
            : public Application
        {
        public:
            NetBaseApplication(int argc, char const **argv);

        protected:
            virtual void OnInit();
            virtual void OnDone();

            std::string GetHost() const;
            std::string GetPort() const;
            std::uint16_t GetWorkers() const;
            std::chrono::microseconds GetTimeout() const;
            std::string GetConfigBranch() const;

        private:
            std::string m_host;
            std::string m_port;
            std::uint16_t m_workers = 0;
            std::uint64_t m_timeout = 0;

            // Application
            virtual void OnStart() override final;
            virtual void OnStop() override final;
        };

    }   // namespace Application
}   // namespace Mif

#endif  // !__MIF_APPLICATION_NET_BASE_APPLICATION_H__
