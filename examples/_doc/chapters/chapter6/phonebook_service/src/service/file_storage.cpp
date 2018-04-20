// STD
#include <fstream>
#include <mutex>
#include <vector>

// BOOST
#include <boost/algorithm/string.hpp>

// MIF
#include <mif/application/iconfig.h>
#include <mif/common/log.h>
#include <mif/service/creator.h>

// THIS
#include "exception.h"
#include "id/service.h"
#include "interface/istorage.h"

namespace Phonebook
{
    namespace Service
    {
        namespace Detail
        {
            namespace
            {

                class Storage
                    : public Mif::Service::Inherit<IStorage>
                {
                public:
                    Storage(Mif::Application::IConfigPtr config)
                    {
                        if (!config)
                            throw std::invalid_argument{"Storage. No config."};

                        m_fileName = config->GetValue("path");

                        std::ifstream file{m_fileName.c_str()};
                        if (!file.is_open())
                            throw std::runtime_error{"Failed to open file \"" + m_fileName + "\" for read."};

                        for (std::string s ; std::getline(file, s) ; )
                        {
                            std::vector<std::string> items;
                            boost::algorithm::split(items, s, boost::is_any_of(";"));
                            if (items.size() < 2)
                                continue;
                            boost::trim(items[0]);
                            boost::trim(items[1]);
                            m_phones.emplace(std::move(items[0]), std::move(items[1]));
                        }
                    }

                    ~Storage()
                    {
                        try
                        {
                            std::ofstream file{m_fileName.c_str()};
                            if (!file.is_open())
                                throw std::runtime_error{"Failed to open file \"" + m_fileName + "\" for write."};

                            for (auto const &i : m_phones)
                                file << i.first << ";" << i.second << std::endl;
                        }
                        catch (std::exception const &e)
                        {
                            MIF_LOG(Error) << "Failed to save data. Error: " << e.what();
                        }
                    }

                private:
                    using LockType = std::mutex;
                    using LockGuard = std::lock_guard<LockType>;

                    std::string m_fileName;

                    mutable LockType m_lock;

                    std::map<std::string, std::string> m_phones;

                    // IStorage
                    virtual std::map<std::string, std::string> List() const override final
                    {
                        std::map<std::string, std::string> phones;

                        {
                            LockGuard lock{m_lock};
                            phones = m_phones;
                        }

                        return phones;
                    }

                    virtual void Set(std::string const &phone, std::string const &name) override final
                    {
                        LockGuard lock{m_lock};
                        m_phones[phone] = name;
                    }

                    virtual std::map<std::string, std::string> Find(std::string const &name) const override final
                    {
                        std::map<std::string, std::string> phones;

                        {
                            LockGuard lock{m_lock};
                            for (auto const &i : m_phones)
                            {
                                if (i.second == name)
                                {
                                    phones.insert(i);
                                    break;
                                }
                            }
                        }

                        return phones;
                    }
                };

            }   // namespace
        }   // namespace Detail
    }   // namespace Service
}   // namespace Phonebook

MIF_SERVICE_CREATOR
(
    ::Phonebook::Id::FileStorage,
    ::Phonebook::Service::Detail::Storage,
    Mif::Application::IConfigPtr
)
