//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_LOG_H__
#define __MIF_COMMON_LOG_H__

// STD
#include <sstream>
#include <utility>

#define MIF_LOG(level_) Logger(::Mif::Common::Log::Level :: level_ )

namespace Mif
{
    namespace Common
    {
        
        class Log final
        {
        public:
            enum Level
            {
                Fatal,
                Error,
                Warning,
                Info,
                Trace
            };

            Log(Log &&) = default;
            Log& operator = (Log &&) = default;
            Log(Log const &) = delete;
            Log& operator = (Log const &) = delete;

            ~Log() noexcept;

            friend Log Logger(Level const &level);

            template <typename T>
            Log& operator << (T && data)
            {
                m_stream << std::forward<T>(data);
                return *this;
            }

        private:
            class Impl;
            Impl &m_impl;
            Level m_level;
            std::stringstream m_stream;

            Log(Level const &level);
        };

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_LOG_H__
