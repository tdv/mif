//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

// STD
#include <istream>
#include <iterator>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

// BOOST
#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>

// MIF
#include "mif/application/iconfig.h"
#include "mif/application/id/config.h"
#include "mif/common/types.h"
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
                    using ValuePtr = std::shared_ptr<boost::json::value>;

                    Collection(boost::json::array const &array, ValuePtr holder)
                        : m_holder{std::move(holder)}
                        , m_array{array}
                        , m_cur{std::begin(m_array)}
                        , m_end{std::end(m_array)}
                    {
                    }

                private:
                    ValuePtr m_holder;
                    boost::json::array const &m_array;
                    boost::json::array::const_iterator m_cur;
                    boost::json::array::const_iterator m_end;

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
                    using ValuePtr = std::shared_ptr<boost::json::value>;

                    Config(StreamPtr stream)
                    {
                        if (!stream)
                            throw std::invalid_argument{"[Mif::Application::Detail::Config] Empty input stream."};


                        Common::Buffer buffer;
                        std::copy(std::istreambuf_iterator<char>(*stream), std::istreambuf_iterator<char>(),
                                std::back_inserter(buffer));

                        m_holder = std::make_shared<boost::json::value>(
                                boost::json::parse({buffer.data(), buffer.size()}));

                        m_root = m_holder.get();
                    }

                    Config(boost::json::value const &root, ValuePtr holder)
                        : m_holder{std::move(holder)}
                        , m_root{&root}
                    {
                    }

                private:
                    ValuePtr m_holder;
                    boost::json::value const *m_root;

                    using StringList = std::list<std::string>;

                    StringList PathToList(std::string path) const
                    {
                        boost::algorithm::trim(path);

                        StringList list;
                        boost::algorithm::split(list, path, boost::algorithm::is_any_of("."));

                        return list;
                    }

                    boost::json::value const* FindItem(std::string const &path) const
                    {
                        auto list = PathToList(path);
                        if (list.empty())
                            return nullptr;

                        auto const *item = m_root->if_object();
                        if (!item)
                            return nullptr;

                        std::size_t size = list.size();
                        std::size_t level = 0;

                        for (auto const &i : list)
                        {
                            if (auto const *nested = item->if_contains(i))
                            {
                                if (auto const *obj = nested->if_object())
                                {
                                    if ((size - level) == 1)
                                        break;

                                    item = obj;
                                    ++level;
                                }
                                else
                                    break;
                            }
                            else
                            {
                                return nullptr;
                            }
                        }

                            return (size - level) == 1 ? item->if_contains(list.back()) : nullptr;
                    }

                    // IConfig
                    virtual bool Exists(std::string const &path) const override final
                    {
                        return !!FindItem(path);
                    }

                    virtual std::string GetValue(std::string const &path) const override final
                    {
                        auto const *item = FindItem(path);
                        if (!item)
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::Config::GetValue] "
                                    "Failed to get value. Bad path \"" + path + "\""};
                        }

                        if (item->is_null())
                            return {};

                        if (auto const *val = item->if_bool())
                            return std::to_string(*val);

                        if (auto const *val = item->if_double())
                            return std::to_string(*val);

                        if (auto const *val = item->if_int64())
                            return std::to_string(*val);

                        if (auto const *val = item->if_uint64())
                            return std::to_string(*val);

                        if (!item->is_string())
                            throw std::bad_alloc{};

                        return boost::json::value_to<std::string>(*item);
                    }

                    virtual Service::TIntrusivePtr<IConfig> GetConfig(std::string const &path) const override final
                    {
                        auto const *item = FindItem(path);
                        if (!item)
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::Config::GetConfig] "
                                    "Failed to get config. Bad path \"" + path + "\""};
                        }

                        if (item->is_null())
                            return {};

                        if (!item->is_object())
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::Config::GetConfig] "
                                    "Failed to get config by path \"" + path + "\". The value is not an object."};
                        }

                        return Service::Make<Config, IConfig>(*item, m_holder);
                    }

                    virtual Common::ICollectionPtr GetCollection(std::string const &path) const override final
                    {
                        auto const *item = FindItem(path);
                        if (!item)
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::Config::GetCollection] "
                                    "Failed to get collection. Bad path \"" + path + "\""};
                        }

                        if (item->is_null())
                            return {};

                        if (!item->is_array())
                        {
                            throw std::invalid_argument{"[Mif::Application::Detail::Config::GetCollection] "
                                    "Failed to get config by path \"" + path + "\". The value is not an object."};
                        }

                        return Service::Make<Collection, Common::ICollection>(item->as_array(), m_holder);
                    }
                };

                Service::IServicePtr Collection::Get()
                {
                    if (IsEmpty() || m_cur == m_end)
                    {
                        throw std::runtime_error{"[Mif::Application::Detail::Collection::Get] "
                                "Failed to get item. No item."};
                    }

                    return Service::Make<Config>(*m_cur, m_holder);
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
