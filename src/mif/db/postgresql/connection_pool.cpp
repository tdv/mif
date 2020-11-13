//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

// STD
#include <cstdint>
#include <exception>
#include <string>

// MIF
#include "mif/application/iconfig.h"
#include "mif/db/iconnection_pool.h"
#include "mif/db/id/service.h"
#include "mif/service/creator.h"
#include "mif/service/factory.h"
#include "mif/service/id/service.h"
#include "mif/service/ipool.h"
#include "mif/service/make.h"

namespace Mif
{
    namespace Db
    {
        namespace PostgreSql
        {
            namespace
            {

                class ConnectionPool
                    : public Service::Inherit<IConnectionPool>
                {
                public:
                    ConnectionPool(std::string const &host, std::uint16_t port, std::string const &user, std::string const &password,
                            std::string const &db, std::uint32_t connectionTimeout)
                    {
                        Init(host, port, user, password, db, connectionTimeout);
                    }

                    ConnectionPool(Application::IConfigPtr config)
                    {
                        if (!config)
                        {
                            throw std::invalid_argument{"[Mif::Db::PostgreSql::ConnectionPool] "
                                    "Empty config ptr."};
                        }

                        Init(config->GetValue("host"),
                             config->GetValue<std::uint16_t>("port"),
                             config->GetValue("user"),
                             config->GetValue("password"),
                             config->GetValue("dbname"),
                             config->GetValue<std::uint32_t>("connectiontimeout")
                            );
                    }

                private:
                    Service::IPoolPtr m_pool;

                    // IConnectionPool
                    virtual IConnectionPtr GetConnection() const override final
                    {
                        return m_pool->GetService<IConnection>();
                    }

                    void Init(std::string const &host, std::uint16_t port, std::string const &user,
                            std::string const &password, std::string const &db,
                            std::uint32_t connectionTimeout)
                    {
                        auto factory = Service::Make<Service::Factory, Service::Factory>();

                        factory->AddClass<Db::Id::Service::PostgreSQL>(
                                std::string{host},
                                std::uint16_t{port},
                                std::string{user},
                                std::string{password},
                                std::string{db},
                                std::uint32_t{connectionTimeout}
                            );

                        m_pool = Service::Create<Service::Id::PerThreadPool, Service::IPool>(
                                Service::IFactoryPtr{factory},
                                Service::ServiceId{Db::Id::Service::PostgreSQL}
                            );
                    }
                };

            }   // namespace
        }   // namespace PostgreSql
    }   // namespace Db
}   // namespace Mif

MIF_SERVICE_CREATOR
(
    Mif::Db::Id::Service::PostgresConnectionPool,
    Mif::Db::PostgreSql::ConnectionPool,
    std::string,
    std::uint16_t,
    std::string,
    std::string,
    std::string,
    std::uint32_t
)

MIF_SERVICE_CREATOR
(
    Mif::Db::Id::Service::PostgresConnectionPool,
    Mif::Db::PostgreSql::ConnectionPool,
    Mif::Application::IConfigPtr
)
