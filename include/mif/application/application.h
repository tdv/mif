//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_APPLICATION_APPLICATION_H__
#define __MIF_APPLICATION_APPLICATION_H__

// STD
#include <string>
#include <type_traits>

// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

namespace Mif
{
    namespace Application
    {

        class Application
        {
        public:

            template <typename T>
            friend
            typename std::enable_if<std::is_base_of<Application, T>::value, int>::type
            Run(int argc, char const **argv)
            {
                return T{argc, argv}.Run();
            }

        protected:
            Application(int argc, char const **argv);

            virtual void OnStart();
            virtual void OnStop();

            int GetArgc() const;
            char const** GetArgv() const;

            void AddCustomOptions(boost::program_options::options_description const &options);
            boost::program_options::variables_map const& GetOptions() const;

            void SetName(std::string const &name);
            std::string const& GetName() const;

            void SetVersion(std::string const &version);
            std::string const& GetVersion() const;

            void SetDescription(std::string const &description);
            std::string const& GetDescription() const;

        private:
            int m_argc;
            char const **m_argv;
            boost::program_options::options_description m_optionsDescr;
            boost::program_options::variables_map m_options;

            std::string m_name;
            std::string m_version;
            std::string m_description;

            Application(Application const &) = delete;
            Application& operator = (Application const &) = delete;
            Application(Application &&) = delete;
            Application& operator = (Application &&) = delete;

            int Run();

            void Start();
            void Stop();
            void LoadConfig();
        };

    }   // namespace Application
}   // namespace Mif

#endif  // !__MIF_APPLICATION_APPLICATION_H__
