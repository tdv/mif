// STD
#include <algorithm>
#include <map>
#include <memory>
#include <mutex>

// BOOST
#include <boost/date_time/posix_time/posix_time.hpp>

// MIF
#include <mif/application/iconfig.h>
#include <mif/common/uuid_generator.h>
#include <mif/service/creator.h>

// THIS
#include "exception.h"
#include "id/service.h"
#include "interface/iadmin.h"
#include "interface/iauth.h"
#include "interface/icache.h"

namespace CacheService
{
    namespace Detail
    {
        namespace
        {

            class DataFacade
                : public Mif::Service::Inherit
                    <
                        IAuth, IAdmin, ICache
                    >
            {
            public:
                DataFacade(Mif::Application::IConfigPtr config)
                {
                    if (!config)
                        throw std::invalid_argument{"DataFacade. No config."};

                    auto rootConfig = config->GetConfig("root");
                    if (!rootConfig)
                        throw std::runtime_error{"DataFacade. No root profile info."};

                    auto login = rootConfig->GetValue("login");;

                    auto root = std::make_shared<Data::Profile>();

                    root->login = login;
                    root->password = rootConfig->GetValue("password");
                    root->name = rootConfig->GetValue("name");
                    root->roles.insert(Data::Role::Admin);

                    m_profiles.emplace(std::move(login), std::move(root));
                }

            private:
                using LockType = std::mutex;
                using LockGuard = std::lock_guard<LockType>;

                using Cache = std::map<std::pair<Data::ID, Data::ID>, std::string>;

                using ProfilePtr = std::shared_ptr<Data::Profile>;
                using Profiles = std::map<Data::ID, ProfilePtr>;

                Mif::Common::UuidGenerator m_uuid;

                boost::posix_time::hours const SessionTTL{48};

                struct SessionInfo
                {
                    Data::Timestamp expires;
                    std::weak_ptr<Data::Profile> profile;
                };

                using Sessions = std::map<Data::ID, SessionInfo>;

                mutable LockType m_authLock;
                mutable Sessions m_sessions;

                mutable LockType m_adminLock;
                Profiles m_profiles;

                mutable LockType m_cacheLock;
                Cache m_cache;

                //IAuth
                virtual Data::Session Login(std::string const &login, std::string const &password) override final
                {
                    ProfilePtr profile;

                    {
                        LockGuard lock{m_adminLock};
                        auto const iter = m_profiles.find(login);
                        if (iter != std::end(m_profiles))
                            profile = iter->second;
                    }

                    if (!profile || profile->password != password)
                        throw Exception::Unauthorized{"Authorization failed."};

                    auto const now = boost::posix_time::second_clock::universal_time();

                    Data::Session session;
                    session.id = m_uuid.Generate();
                    session.expires = now + SessionTTL;

                    {
                        SessionInfo info;
                        info.expires = session.expires;
                        info.profile = profile;

                        LockGuard lock{m_authLock};
                        m_sessions.emplace(session.id, std::move(info));
                    }

                    return session;
                }

                virtual void Logout(Data::ID const &session) override final
                {
                    LockGuard lock{m_authLock};
                    auto iter = m_sessions.find(session);
                    if (iter == std::end(m_sessions))
                        throw std::invalid_argument{"Session \"" + session + "\" not found."};
                    m_sessions.erase(iter);
                }

                virtual Data::Profile GetSessionProfile(Data::ID const &session) const override final
                {
                    ProfilePtr profile;

                    {
                        LockGuard lock{m_authLock};
                        auto iter = m_sessions.find(session);
                        if (iter == std::end(m_sessions))
                            throw Exception::Unauthorized{"Session \"" + session + "\" not found."};
                        profile = iter->second.profile.lock();
                        if (!profile)
                        {
                            m_sessions.erase(iter);
                            throw Exception::Unauthorized{"Session \"" + session + "\" not found."};
                        }
                    }

                    return *profile;
                }

                //IAdmin
                virtual Data::StringMap GetProfilesList() const override final
                {
                    Data::StringMap profiles;

                    {
                        LockGuard lock{m_adminLock};
                        std::transform(std::begin(m_profiles), std::end(m_profiles),
                                std::inserter(profiles, std::begin(profiles)),
                                [] (typename std::decay<decltype(m_profiles)>::type::value_type const &i)
                                {
                                    return std::make_pair(i.second->login, i.second->name);
                                }
                            );
                    }

                    return profiles;
                }

                virtual Data::Profile GetProfile(std::string const &login) const override final
                {
                    Data::Profile profile;

                    {
                        LockGuard lock{m_adminLock};
                        auto const iter = m_profiles.find(login);
                        if (iter == std::end(m_profiles))
                            throw std::invalid_argument{"Profile with login \"" + login + "\" not found."};
                        profile = *iter->second;
                    }

                    return profile;
                }

                virtual void SetProfile(Data::Profile const &profile) override final
                {
                    LockGuard lock{m_adminLock};
                    m_profiles[profile.login] = std::make_shared<Data::Profile>(profile);
                }

                virtual void RemoveProfile(std::string const &login) override final
                {
                    LockGuard lock{m_adminLock};
                    auto iter = m_profiles.find(login);
                    if (iter == std::end(m_profiles))
                        throw std::invalid_argument{"Profile with login \"" + login + "\" not found."};
                    m_profiles.erase(iter);
                }

                // ICache
                virtual std::map<Data::ID, Data::IDs> ListBucketKeys(Data::IDs const &buckets) const override final
                {
                    std::map<Data::ID, Data::IDs> ret;

                    {
                        LockGuard lock{m_cacheLock};
                        for (auto const &i : m_cache)
                        {
                            if (buckets.find(i.first.first) != std::end(buckets))
                                ret[i.first.first].insert(i.first.second);
                        }
                    }

                    return ret;
                }

                virtual std::string GetData(Data::ID const &bucket, Data::ID const &key) const override final
                {
                    std::string data;

                    {
                        LockGuard lock{m_cacheLock};
                        auto const iter = m_cache.find(std::make_pair(bucket, key));
                        if (iter == std::end(m_cache))
                            throw Exception::NotFound{"Data for bucket \"" + bucket + "\" and key \"" + key + "\" not found."};
                        data = iter->second;
                    }

                    return data;
                }

                virtual void SetData(Data::ID const &bucket, Data::ID const &key,
                        std::string const &data) override final
                {
                    LockGuard lock{m_cacheLock};
                    m_cache[std::make_pair(bucket, key)] = data;
                }

                virtual void RemoveData(Data::ID const &bucket, Data::ID const &key) override final
                {
                    LockGuard lock{m_cacheLock};
                    auto iter = m_cache.find(std::make_pair(bucket, key));
                    if (iter == std::end(m_cache))
                        throw Exception::NotFound{"Data for bucket \"" + bucket + "\" and key \"" + key + "\" not found."};
                    m_cache.erase(iter);
                }
            };

        }   // namespace
    }   // namespace Detail
}   // namespace CacheService

MIF_SERVICE_CREATOR
(
    ::CacheService::Id::DataFacade,
    ::CacheService::Detail::DataFacade,
    Mif::Application::IConfigPtr
)
