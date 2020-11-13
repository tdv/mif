//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_APPLICATION_APPLICATION_H__
#define __MIF_APPLICATION_APPLICATION_H__

// STD
#include <memory>
#include <string>
#include <type_traits>

// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

// MIF
#include "mif/application/iconfig.h"

namespace Mif
{
    namespace Application
    {

        class Application;

        template <typename T>
        typename std::enable_if<std::is_base_of<Application, T>::value, int>::type
        Run(int argc, char const **argv);

        class Application
        {
        protected:
            Application(int argc, char const **argv);
            virtual ~Application();

            virtual void OnStart();
            virtual void OnStop();

            int GetArgc() const;
            char const** GetArgv() const;

            void AddCustomOptions(boost::program_options::options_description const &options);
            boost::program_options::variables_map const& GetOptions() const;

            virtual void PrepareConfigData(std::string &data, std::string const &format) const;
            std::string const& GetConfigRawData() const;
            IConfigPtr GetConfig() const;

            void SetName(std::string const &name);
            std::string const& GetName() const;

            void SetVersion(std::string const &version);
            std::string const& GetVersion() const;

            void SetDescription(std::string const &description);
            std::string const& GetDescription() const;

        private:
            int m_argc;
            char const **m_argv;

            bool m_runAsDaemon = false;
            std::string m_pidFileName;
            std::string m_configFileName;
            std::string m_configFileFormat;
            std::string m_logDirName;
            std::string m_logPattern;
            std::uint32_t m_logLevel;

            boost::program_options::options_description m_optionsDescr;
            boost::program_options::variables_map m_options;

            std::string m_version;
            std::string m_name;
            std::string m_description;

            std::string m_configRawData;
            IConfigPtr m_config;

            class Daemon;
            std::unique_ptr<Daemon> m_daemon;

            Application(Application const &) = delete;
            Application& operator = (Application const &) = delete;
            Application(Application &&) = delete;
            Application& operator = (Application &&) = delete;

            template <typename T>
            friend
            typename std::enable_if<std::is_base_of<Application, T>::value, int>::type
            Run(int argc, char const **argv)
            {
                return T{argc, argv}.Run();
            }

            int Run();

            void InitLog(bool pathFromConfig, bool levelFromConfig);

            void Start();
            void Stop();
            IConfigPtr LoadConfig() const;
            void RunAsDaemon();
            void RunInThisProcess();
        };

    }   // namespace Application
}   // namespace Mif

#endif  // !__MIF_APPLICATION_APPLICATION_H__
