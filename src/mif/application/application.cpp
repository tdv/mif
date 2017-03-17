//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <thread>

// BOOST
#include <boost/filesystem/path.hpp>
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

        Application::Application(int argc, char const **argv)
            : m_argc{argc}
            , m_argv{argv}
            , m_optionsDescr{"Allowed options"}
            , m_name{boost::filesystem::path{argv[0]}.filename().c_str()}
            , m_description{"MIF application"}
        {
            m_optionsDescr.add_options()
                    ("help,h", "Show help.")
                    ("version,v", "Show program version.")
            #if defined(__linux__) || defined(__unix__)
                    ("daemon,d", "Run as daemon.")
            #endif
                    ("config,c", boost::program_options::value<std::string>(), "Config file name (full path).");
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

                Start();
            }
            catch (std::exception const &e)
            {
                MIF_LOG(Fatal) << "[Mif::Application::Application::Run] Failed to start application. Error: " << e.what();
            }
            return EXIT_FAILURE;
        }

        void Application::Start()
        {
            bool RunAsDaemon = false;

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
                    RunAsDaemon = true;
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
                if (!RunAsDaemon)
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
                else
                {
                    // TODO: run as daemon
                }
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

    }   // namespace Application
}   // namespace Mif
