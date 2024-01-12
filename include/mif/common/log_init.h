//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_LOG_INIT_H__
#define __MIF_COMMON_LOG_INIT_H__

// STD
#include <cstdint>
#include <string>

// MIF
#include "mif/common/log.h"

namespace Mif
{
    namespace Common
    {

        void InitConsoleLog(Log::Level const &level = Log::Level::Trace);

        void InitFileLog(Log::Level const &level,
            std::string const &logDir, std::string const &filePattern,
            std::size_t maxSize = 1, std::size_t maxCount = 10);

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_LOG_INIT_H__
