//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// EVENT
#include <event2/event.h>

// MIF
#include "mif/common/log.h"
#include "mif/common/unused.h"

// THIS
#include "lib_event_initializer.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                void LibEventInitializer::Init()
                {
                    static LibEventInitializer instance;
                    Common::Unused(instance);
                }

                LibEventInitializer::LibEventInitializer()
                {
                    event_set_fatal_callback(&LibEventInitializer::OnFatal);
                    event_set_log_callback(&LibEventInitializer::LogMessage);
                }

                void LibEventInitializer::OnFatal(int err)
                {
                    MIF_LOG(Fatal) << "[Mif::Net::Http] LibEvent fatal error. Code: " << err;
                }

                void LibEventInitializer::LogMessage(int severity, char const *message)
                {
                    auto level = Common::Log::Level::Info;

                    switch (severity)
                    {
                    case EVENT_LOG_ERR :
                        level = Common::Log::Level::Error ;
                        break;
                    case EVENT_LOG_WARN :
                        level = Common::Log::Level::Warning ;
                        break;
                    case EVENT_LOG_MSG :
                        level = Common::Log::Level::Info ;
                        break;
                    case EVENT_LOG_DEBUG :
                        level = Common::Log::Level::Trace ;
                        break;
                    default :
                        break;
                    }

                    Logger(level) << "[Mif::Net::Http] LibEvent. " << (message ? message : "");
                }

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
