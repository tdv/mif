//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <istream>
#include <memory>
#include <stdexcept>

// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

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
                    Collection(boost::property_tree::ptree const &array)
                        : m_array{array}
                        , m_cur{std::begin(m_array)}
                        , m_end{std::end(m_array)}
                    {
                    }

                private:
                    boost::property_tree::ptree m_array;
                    boost::property_tree::ptree::const_iterator m_cur;
                    boost::property_tree::ptree::const_iterator m_end;

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

                        try
                        {
                            boost::property_tree::ptree tree;
                            boost::property_tree::xml_parser::read_xml(*stream, tree);
                            if (auto document = tree.get_child_optional("document"))
                                m_tree = document.get();
                        }
                        catch (std::exception const &e)
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::Config] Empty input stream. "
                                "Failed to parse xml. Error: " + std::string{e.what()}};
                        }
                    }

                    Config(boost::property_tree::ptree const &tree)
                        : m_tree{tree}
                    {
                    }

                private:
                    boost::property_tree::ptree m_tree;

                    // IConfig
                    virtual bool Exists(std::string const &path) const override final
                    {
                        return m_tree.find(path) != m_tree.not_found();
                    }

                    virtual std::string GetValue(std::string const &path) const override final
                    {
                        try
                        {
                            if (auto value = m_tree.get_optional<std::string>(path))
                                return value.get();
                        }
                        catch (std::exception const &e)
                        {
                            throw std::runtime_error{"[Mif::Application::Detail::Config::GetValue] "
                                    "Failed to get value by path \"" + path + "\". Error: " + std::string{e.what()}};
                        }

                        throw std::invalid_argument{"[Mif::Application::Detail::Config::GetValue] "
                                "Failed to get value. Bad path \"" + path + "\""};
                    }

                    virtual Service::TIntrusivePtr<IConfig> GetConfig(std::string const &path) const override final
                    {
                        try
                        {
                            if (auto config = m_tree.get_child_optional(path))
                                return Service::Make<Config, IConfig>(config.get());
                        }
                        catch (std::exception const &e)
                        {
                            throw std::runtime_error{"[Mif::Application::Detail::Config::GetConfig] "
                                    "Failed to get config by path \"" + path + "\". Error: " + std::string{e.what()}};
                        }

                        throw std::invalid_argument{"[Mif::Application::Detail::Config::GetConfig] "
                                "Failed to get config. Bad path \"" + path + "\""};
                    }

                    virtual Common::ICollectionPtr GetCollection(std::string const &path) const override final
                    {
                        try
                        {
                            if (auto array = m_tree.get_child_optional(path))
                                return Service::Make<Collection, Common::ICollection>(array.get());
                        }
                        catch (std::exception const &e)
                        {
                            throw std::runtime_error{"[Mif::Application::Detail::Config::GetCollection] "
                                    "Failed to get collection by path \"" + path + "\". Error: " + std::string{e.what()}};
                        }

                        throw std::invalid_argument{"[Mif::Application::Detail::Config::GetCollection] "
                                "Failed to get collection. Bad path \"" + path + "\""};
                    }
                };

                Service::IServicePtr Collection::Get()
                {
                    if (IsEmpty() || m_cur == m_end)
                    {
                        throw std::runtime_error{"[Mif::Application::Detail::Collection::Get] "
                                "Failed to get item. No item."};
                    }

                    return Service::Make<Config>(m_cur->second);
                }

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
