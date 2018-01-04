//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// STD
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <stdexcept>
#include <string>

// BOOST
#include <boost/log/attributes.hpp>
#include <boost/log/core/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

// MIF
#include "mif/common/log.h"
#include "mif/common/log_init.h"

namespace
{

    enum MifCommonLogLevel{};

    std::ostream& operator << (std::ostream &stream, MifCommonLogLevel const &level)
    {
        switch (static_cast<std::uint32_t>(level))
        {
        case Mif::Common::Log::Level::Fatal :
            stream << "FATAL";
            break;
        case Mif::Common::Log::Level::Error :
            stream << "ERROR";
            break;
        case Mif::Common::Log::Level::Warning :
            stream << "WARNING";
            break;
        case Mif::Common::Log::Level::Info :
            stream << "INFO";
            break;
        case Mif::Common::Log::Level::Trace :
            stream << "TRACE";
            break;
        default :
            stream << "UNKNOWN";
            break;
        }
        return stream;
    }

}   // namespace

namespace Mif
{
    namespace Common
    {
        namespace Detail
        {
            namespace
            {

                BOOST_LOG_ATTRIBUTE_KEYWORD(CustomSeverity, "Severity", MifCommonLogLevel)

                class Logger final
                {
                public:
                    using Level = Log::Level;

                    


                    static Logger& Get()
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        auto *instance = m_instance.get();
                        if (!instance)
                            InitConsoleLogInternal(Log::Level::Trace);
                        instance = m_instance.get();
                        if (!instance)
                            throw std::runtime_error{"[Mif::Common::Detail::Logger::Get] Log was not initialized."};

                        return *instance;
                    }

                    static void InitConsoleLog(Log::Level const &level)
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        InitConsoleLogInternal(level);
                    }
                
                    static void InitFileLog(Log::Level const &level,
                        std::string const &logDir, std::string const &filePattern,
                        std::size_t maxSize, std::size_t maxCount)
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        if (m_instance)
                            throw std::runtime_error{"[Mif::Common::Detail::Logger::Init] Already initialized."};

                        m_instance.reset(new Logger(level, logDir, filePattern, maxSize, maxCount));
                    }

                    Logger(Logger const &) = delete;
                    Logger& operator = (Logger const &) = delete;
                    Logger(Logger &&) = delete;
                    Logger& operator = (Logger &&) = delete;

                    void PutMessage(Level const &level, std::string const &message)
                    {
                        auto logLevel = static_cast<MifCommonLogLevel>(static_cast<std::uint32_t>(level));
                        BOOST_LOG_SEV(m_logger, logLevel) << message;
                    }

                private:
                    boost::log::sources::severity_logger_mt<MifCommonLogLevel> m_logger;

                    static std::mutex m_mutex;
                    static std::unique_ptr<Logger> m_instance;

                    static void InitConsoleLogInternal(Log::Level const &level)
                    {
                        if (m_instance)
                            throw std::runtime_error{"[Mif::Common::Detail::Logger::Init] Already initialized."};

                        m_instance.reset(new Logger(level));
                    }

#define MIF_LOG_RECORD_FORMAT \
    expressions::stream \
        << expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%dT%H:%M:%S.%f") \
        << " [" << CustomSeverity << "]" \
        << ": " << expressions::message

                    Logger(Log::Level const &level)
                    {
                        using namespace boost::log;
                        Init(level);
                        boost::log::add_console_log(std::clog,
                            boost::log::keywords::format = MIF_LOG_RECORD_FORMAT);
                    }

                    Logger(Log::Level const &level, std::string const &logDir, std::string const &filePattern,
                        std::size_t maxSize, std::size_t maxCount)
                    {
                        using namespace boost::log;
                        Init(level);
                        auto fullPath = boost::filesystem::path(logDir) / filePattern;
                        add_file_log
                            (
                                keywords::target = logDir,
                                keywords::file_name = fullPath,
                                keywords::rotation_size = maxSize * 1024 * 1024,
                                keywords::max_size = maxCount * 1024 * 1024,
                                keywords::scan_method = sinks::file::scan_matching,
                                keywords::auto_flush = true,
                                keywords::format = MIF_LOG_RECORD_FORMAT
                            );
                    }

#undef MIF_LOG_RECORD_FORMAT

                    void Init(Log::Level const &level)
                    {
                        using namespace boost::log;
                        auto core = core::get();
                        core->set_filter(CustomSeverity <= level);
                        core->add_global_attribute("TimeStamp", attributes::local_clock());
                    }
                };

                std::mutex Logger::m_mutex;
                std::unique_ptr<Logger> Logger::m_instance;

            }   // namespace
        }   // namespace Detail
        
        Log Logger(Log::Level const &level)
        {
            return Log{level};
        }

        class Log::Impl final
        {
        public:
            static Impl& Get()
            {
                static Impl impl;
                return impl;
            }

            Impl(Impl const &) = delete;
            Impl& operator = (Impl const &) = delete;
            Impl(Impl &&) = delete;
            Impl& operator = (Impl &&) = delete;

            void PutMessage(Log::Level const &level, std::string const &message)
            {
                Detail::Logger::Get().PutMessage(level, message);
            }

        private:
            Impl() = default;
        };

        Log::Log(Level const &level)
            : m_impl(Impl::Get())
            , m_level(level)
        {
        }

        Log::~Log() noexcept
        {
            try
            {
                m_impl.PutMessage(m_level, m_stream.str());
            }
            catch (std::exception const &e)
            {
                std::clog << "[Mif::Common::Log::~Log] Failed to put message. Error: " << e.what() << std::endl;
            }
        }

        void InitConsoleLog(Log::Level const &level)
        {
            Detail::Logger::InitConsoleLog(level);
        }

        void InitFileLog(Log::Level const &level,
            std::string const &logDir, std::string const &filePattern,
            std::size_t maxSize, std::size_t maxCount)
        {
            Detail::Logger::InitFileLog(level, logDir, filePattern, maxSize, maxCount);
        }

    }   // namespace Common
}   // namespace Mif
