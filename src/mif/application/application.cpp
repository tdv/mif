//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// C
#if defined(__linux__) || defined(__unix__)
#include <unistd.h>
#include <signal.h>
#endif

// STD
#include <condition_variable>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>

// BOOST
//#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options/parsers.hpp>

// MIF
#include "mif/application/application.h"
#include "mif/common/log.h"
#include "mif/common/log_init.h"
#include "mif/service/root_locator.h"

namespace Mif
{
    namespace Application
    {

        class Application::Daemon final
        {
        public:
        #if defined(__linux__) || defined(__unix__)
            Daemon(std::function<void ()> onStart, std::function<void ()> onStop)
            {
                if (daemon(0, 0))
                    throw std::runtime_error{"[Mif::Application::Daemon] The application could not be started in daemon mode."};

                MIF_LOG(Info) << "[Mif::Application::Daemon] Starting application in a daemon mode...";

                try
                {
                    onStart();
                }
                catch (std::exception const &e)
                {
                    MIF_LOG(Fatal) << "[Mif::Application::Daemon] Failed to start application. Error: " << e.what();
                }
                catch (...)
                {
                    MIF_LOG(Fatal) << "[Mif::Application::Daemon] Failed to start application. Error: unknown error.";
                }

                signal(SIGINT, ShutdownSignal);
                signal(SIGQUIT, ShutdownSignal);
                signal(SIGTERM, ShutdownSignal);

                signal(SIGPIPE, SIG_IGN);

                m_mutex.reset(new std::mutex);
                m_cv.reset(new std::condition_variable);

                std::unique_lock<std::mutex> lock{*m_mutex};

                MIF_LOG(Info) << "[Mif::Application::Daemon] The application is started in daemon mode.";

                m_cv->wait(lock);

                MIF_LOG(Info) << "[Mif::Application::Daemon] Stopping daemon ...";

                m_cv.reset();
                m_mutex.reset();

                try
                {
                    onStop();
                }
                catch (std::exception const &e)
                {
                    MIF_LOG(Warning) << "[Mif::Application::Daemon] Failed to correct stop application. Error: " << e.what();
                }
                catch (...)
                {
                    MIF_LOG(Warning) << "[Mif::Application::Daemon] Failed to correct stop application. Error: unknown error.";
                }

                MIF_LOG(Info) << "[Mif::Application::Daemon] Stopped daemon.";
            }
        private:
            static std::unique_ptr<std::mutex> m_mutex;
            static std::unique_ptr<std::condition_variable> m_cv;

            static void ShutdownSignal(int)
            {
                if (m_cv)
                    m_cv->notify_all();
            }
        #endif
        };

        #if defined(__linux__) || defined(__unix__)
        std::unique_ptr<std::mutex> Application::Daemon::m_mutex;
        std::unique_ptr<std::condition_variable> Application::Daemon::m_cv;
        #endif

        Application::Application(int argc, char const **argv)
            : m_argc{argc}
            , m_argv{argv}
            , m_logLevel{Common::Log::Level::Trace}
            , m_optionsDescr{"Allowed options"}
            , m_name{boost::filesystem::path{argv[0]}.filename().c_str()}
            , m_description{"MIF application"}
        {
            m_optionsDescr.add_options()
                    ("help,h", "Show help.")
                    ("version,v", "Show program version.")
            #if defined(__linux__) || defined(__unix__)
                    ("daemon,d", "Run as daemon.")
                    ("pidfile,p", boost::program_options::value<std::string>(&m_pidFileName), "Path to pid-file.")
            #endif
                    ("config,c", boost::program_options::value<std::string>(&m_configFileName), "Config file name (full path).")
                    ("logdir", boost::program_options::value<std::string>(&m_logDirName)->default_value(
                            boost::filesystem::canonical(boost::filesystem::path{argv[0]}.parent_path()).c_str()), "Log directory name.")
                    ("logpattern", boost::program_options::value<std::string>(&m_logPattern)->default_value(
                            m_name + "_%5N.log"), "Log file pattern.")
                    ("loglevel", boost::program_options::value<std::uint32_t>(&m_logLevel)->default_value(Common::Log::Level::Trace), "Log level.");
        }

        Application::~Application()
        {
        }

        void Application::OnStart()
        {
        }

        void Application::OnStop()
        {
        }

        int Application::GetArgc() const
        {
            return m_argc;
        }

        char const** Application::GetArgv() const
        {
            return m_argv;
        }

        void Application::AddCustomOptions(boost::program_options::options_description const &options)
        {
            m_optionsDescr.add(options);
        }

        boost::program_options::variables_map const& Application::GetOptions() const
        {
            return m_options;
        }

        void Application::SetName(std::string const &name)
        {
            m_name = name;
        }

        std::string const& Application::GetName() const
        {
            return m_name;
        }

        void Application::SetVersion(std::string const &version)
        {
            m_version = version;
        }

        std::string const& Application::GetVersion() const
        {
            return m_version;
        }

        void Application::SetDescription(std::string const &description)
        {
            m_description = description;
        }

        std::string const& Application::GetDescription() const
        {
            return m_description;
        }

        int Application::Run()
        {
            try
            {
                boost::program_options::store(
                        boost::program_options::parse_command_line(m_argc, m_argv, m_optionsDescr),
                        m_options
                    );

                boost::program_options::notify(m_options);

                if (m_options.count("help"))
                {
                    std::cout << m_optionsDescr << std::endl;
                    return EXIT_SUCCESS;
                }

                if (m_options.count("version"))
                {
                    std::cout << "Application: " << m_name << std::endl
                              << "Veraion: " << m_version << std::endl
                              << "Description: " << m_description << std::endl;
                    return EXIT_SUCCESS;
                }

                ProcessLogOptions();

                Start();
            }
            catch (std::exception const &e)
            {
                MIF_LOG(Fatal) << "[Mif::Application::Application::Run] Failed to start application. Error: " << e.what();
            }
            return EXIT_FAILURE;
        }

        void Application::ProcessLogOptions()
        {
            if (m_options.count("loglevel"))
            {
                if (m_logLevel > Common::Log::Level::Trace)
                {
                    throw std::invalid_argument{"[Mif::Application::Application::Run] "
                            "Invalid log level valie \"" + std::to_string(m_logLevel) + "\". "
                            "The value of the logging level should not exceed \"" + std::to_string(Common::Log::Level::Trace) + "\"."};
                }
            }

            if (m_options.count("logdir") || m_options.count("logpattern"))
            {
                if (!boost::filesystem::exists(m_logDirName))
                {
                    if (!boost::filesystem::create_directories(m_logDirName))
                    {
                        throw std::runtime_error{"[Mif::Application::Application::Run] "
                                "Failed to create log directory \"" + m_logDirName + "\""};
                    }
                }
                Common::InitFileLog(static_cast<Common::Log::Level>(m_logLevel), m_logDirName, m_logPattern);
            }
            else if (m_options.count("loglevel"))
            {
                Common::InitConsoleLog(static_cast<Common::Log::Level>(m_logLevel));
            }
        }

        void Application::Start()
        {
            bool NeedRunAsDaemon = false;

            auto locator = Service::RootLocator::Get();

            try
            {
                if (m_options.count("config"))
                {
                    LoadConfig();
                    // TODO: set RunAsDaemon from config
                }

            #if defined(__linux__) || defined(__unix__)
                if (m_options.count("daemon"))
                    NeedRunAsDaemon = true;
            #endif
            }
            catch (std::exception const &e)
            {
                locator->Clear();
                MIF_LOG(Fatal) << "[Mif::Application::Application::Start] Failed to start application. Error: " << e.what();
                throw;
            }

            try
            {
                if (!NeedRunAsDaemon)
                    RunInThisProcess();
                else
                    RunAsDaemon();
            }
            catch (std::exception const &e)
            {
                locator->Clear();
                MIF_LOG(Fatal) << "[Mif::Application::Application::Start] Failed to call OnStart. Error: " << e.what();
                throw;
            }
        }

        void Application::Stop()
        {
            try
            {
                OnStop();
            }
            catch (std::exception const &e)
            {
                Service::RootLocator::Get()->Clear();
                MIF_LOG(Warning) << "[Mif::Application::Application::Stop] Failed to call OnStop. Error: " << e.what();
            }
        }

        void Application::LoadConfig()
        {
            ;
        }

        void Application::RunAsDaemon()
        {
            #if defined(__linux__) || defined(__unix__)

            if (!m_daemon)
                m_daemon.reset(new Daemon{ [this] { OnStart(); }, [this] { OnStop(); } });

            #endif
        }

        void Application::RunInThisProcess()
        {
            std::exception_ptr exception{};

            std::thread t{
                    [this, &exception]
                    {
                        try
                        {
                            OnStart();
                            std::cout << "Press 'Enter' for quit." << std::endl;
                            std::cin.get();
                            Stop();
                        }
                        catch (...)
                        {
                            exception = std::current_exception();
                        }
                    }
                };

            t.join();

            if (exception)
                std::rethrow_exception(exception);
        }

    }   // namespace Application
}   // namespace Mif
