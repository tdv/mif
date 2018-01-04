//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// STD
#include <istream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

// BOOST
#include <boost/algorithm/string.hpp>

// JSONCPP
#include <json/json.h>
#include <json/reader.h>

// MIF
#include "mif/application/iconfig.h"
#include "mif/application/id/config.h"
#include "mif/service/creator.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Application
    {
        namespace Detail
        {
            namespace
            {

                class Collection
                    : public Service::Inherit<Common::ICollection>
                {
                public:
                    Collection(::Json::Value array)
                        : m_array{std::move(array)}
                        , m_cur{std::begin(m_array)}
                        , m_end{std::end(m_array)}
                    {
                    }

                private:
                    ::Json::Value m_array;
                    ::Json::Value::const_iterator m_cur;
                    ::Json::Value::const_iterator m_end;

                    // ICollection
                    virtual bool Next() override final
                    {
                        if (m_cur != m_end)
                            ++m_cur;

                        return m_cur != m_end;
                    }

                    virtual bool IsEmpty() const override final
                    {
                        return m_array.empty();
                    }

                    virtual void Reset() override final
                    {
                        m_cur = std::begin(m_array);
                    }

                    virtual Service::IServicePtr Get() override final;
                };

                class Config
                    : public Service::Inherit<IConfig>
                {
                public:
                    using StreamPtr = std::shared_ptr<std::istream>;

                    Config(StreamPtr stream)
                    {
                        if (!stream)
                            throw std::invalid_argument{"[Mif::Application::Detail::Config] Empty input stream."};

                        ::Json::Reader reader;
                        if (!reader.parse(*stream, m_root))
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::Config] Empty input stream. "
                                "Failed to parse json. Error: " + reader.getFormattedErrorMessages()};
                        }
                    }

                    Config(::Json::Value const &root)
                        : m_root{root}
                    {
                    }

                private:
                    ::Json::Value m_root;

                    using StringList = std::list<std::string>;

                    StringList PathToList(std::string path) const
                    {
                        boost::algorithm::trim(path);

                        StringList list;
                        boost::algorithm::split(list, path, boost::algorithm::is_any_of("."));

                        return list;
                    }

                    bool FindItem(std::string const &path, ::Json::Value &value) const
                    {
                        StringList const list = PathToList(path);

                        ::Json::Value item = m_root;

                        for (auto const &i : list)
                        {
                            if (!item.isMember(i))
                                return false;
                            item = item.get(i, ::Json::Value{});
                        }

                        std::swap(item, value);

                        return true;
                    }

                    // IConfig
                    virtual bool Exists(std::string const &path) const override final
                    {
                        ::Json::Value item;
                        return FindItem(path, item);
                    }

                    virtual std::string GetValue(std::string const &path) const override final
                    {
                        ::Json::Value item;
                        if (!FindItem(path, item))
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::Config::GetValue] "
                                    "Failed to get value. Bad path \"" + path + "\""};
                        }

                        if (item.type() == ::Json::nullValue)
                            return {};

                        if (item.type() != ::Json::stringValue && !item.isConvertibleTo(::Json::stringValue))
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::Config::GetValue] "
                                    "Failed to get value by path \"" + path + "\". Can not convert to a string type."};
                        }

                        return item.asString();
                    }

                    virtual Service::TIntrusivePtr<IConfig> GetConfig(std::string const &path) const override final
                    {
                        ::Json::Value item;
                        if (!FindItem(path, item))
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::Config::GetConfig] "
                                    "Failed to get config. Bad path \"" + path + "\""};
                        }

                        if (item.type() == ::Json::nullValue)
                            return {};

                        if (!item.isObject())
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::Config::GetConfig] "
                                    "Failed to get config by path \"" + path + "\". The value is not an object."};
                        }

                        return Service::Make<Config, IConfig>(item);
                    }

                    virtual Common::ICollectionPtr GetCollection(std::string const &path) const override final
                    {
                        ::Json::Value item;
                        if (!FindItem(path, item))
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::Config::GetCollection] "
                                    "Failed to get collection. Bad path \"" + path + "\""};
                        }

                        if (item.type() == ::Json::nullValue)
                            return {};

                        if (!item.isArray())
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::Config::GetCollection] "
                                    "Failed to get config by path \"" + path + "\". The value is not an object."};
                        }

                        return Service::Make<Collection, Common::ICollection>(std::move(item));
                    }
                };

                Service::IServicePtr Collection::Get()
                {
                    if (IsEmpty() || m_cur == m_end)
                    {
                        throw std::runtime_error{"[Mif::Application::Detail::Collection::Get] "
                                "Failed to get item. No item."};
                    }

                    return Service::Make<Config>(*m_cur);
                }

            }   // namespace
        }   // namespace Detail
    }   // namespace Application
}   // namespace Mif

MIF_SERVICE_CREATOR
(
    Mif::Application::Id::Service::Config::Json,
    Mif::Application::Detail::Config,
    Mif::Application::Detail::Config::StreamPtr
)
