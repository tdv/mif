//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <fstream>
#include <memory>

// MIF
#include "mif/application/iconfig.h"
#include "mif/application/id/config.h"
#include "mif/service/creator.h"

namespace Mif
{
    namespace Application
    {
        namespace Detail
        {
            namespace
            {

                class Config
                    : public Service::Inherit<IConfig>
                {
                public:
                    using StreamPtr = std::shared_ptr<std::ifstream>;

                    Config(StreamPtr stream)
                    {
                        (void)stream;
                    }

                private:
                    // IConfig
                    virtual bool Exists(std::string const &path) const override final
                    {
                        (void)path;
                        return {};
                    }

                    virtual std::string GetValue(std::string const &path) const override final
                    {
                        (void)path;
                        return {};
                    }

                    virtual Service::TIntrusivePtr<IConfig> GetConfig(std::string const &path) const override final
                    {
                        (void)path;
                        return {};
                    }

                    virtual Common::ICollectionPtr GetCollection(std::string const &path) const override final
                    {
                        (void)path;
                        return {};
                    }
                };

            }   // namespace
        }   // namespace Detail
    }   // namespace Application
}   // namespace Mif

MIF_SERVICE_CREATOR
(
    Mif::Application::Id::Service::Config::Xml,
    Mif::Application::Detail::Config,
    Mif::Application::Detail::Config::StreamPtr
)
