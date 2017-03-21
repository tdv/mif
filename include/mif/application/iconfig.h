//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_APPLICATION_ICONFIG_H__
#define __MIF_APPLICATION_ICONFIG_H__

// STD
#include <sstream>
#include <string>

// MIF
#include "mif/common/icollection.h"
#include "mif/service/iservice.h"

namespace Mif
{
    namespace Application
    {
        struct IConfig
            : public Service::Inherit<Service::IService>
        {
            virtual bool Exists(std::string const &path) const = 0;
            virtual std::string GetValue(std::string const &path) const = 0;
            virtual Service::TIntrusivePtr<IConfig> GetConfig(std::string const &path) const = 0;
            virtual Common::ICollectionPtr GetCollection(std::string const &path) const = 0;

            template <typename T>
            T GetValue(std::string const &path) const
            {
                std::stringstream stream{GetValue(path)};
                T value{};
                stream >> value;
                return value;
            }
        };

        using IConfigPtr = Service::TServicePtr<IConfig>;

    }   // namespace Application
}   // namespace Mif

#endif  // !__MIF_APPLICATION_ICONFIG_H__
