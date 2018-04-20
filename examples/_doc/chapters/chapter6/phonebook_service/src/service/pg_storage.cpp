// BOOST
#include <boost/algorithm/string.hpp>

// MIF
#include <mif/application/iconfig.h>
#include <mif/common/unused.h>
#include <mif/db/iconnection_pool.h>
#include <mif/db/id/service.h>
#include <mif/db/transaction.h>
#include <mif/service/create.h>
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

                        m_connections = Mif::Service::Create
                                <
                                    Mif::Db::Id::Service::PostgresConnectionPool,
                                    Mif::Db::IConnectionPool
                                >(config->GetConfig("database"));
                    }

                private:
                    Mif::Db::IConnectionPoolPtr m_connections;

                    std::string Quote(std::string const &str) const
                    {
                        std::string tmp = str;
                        boost::replace_all(tmp, "'", "''");
                        return "'" + tmp + "'";
                    }

                    // IStorage
                    virtual std::map<std::string, std::string> List() const override final
                    {
                        auto conn = m_connections->GetConnection();
                        Mif::Db::Transaction tr{conn};
                        Mif::Common::Unused(tr);

                        auto stmt = conn->CreateStatement("select phone, name from phones order by name;");
                        auto res = stmt->Execute();

                        std::map<std::string, std::string> phones;

                        while (res->Read())
                        {
                            std::size_t index = 0;

                            auto phone = res->GetAsString(index++);
                            auto name = res->GetAsString(index++);

                            phones.emplace(std::move(phone), std::move(name));
                        }

                        return phones;
                    }

                    virtual void Set(std::string const &phone, std::string const &name) override final
                    {
                        auto conn = m_connections->GetConnection();
                        Mif::Db::Transaction tr{conn};

                        auto stmt = conn->CreateStatement(
                                "insert into phones (phone, name) "
                                "values ($1::text, $2::text) "
                                "returning phone; "
                            );

                        auto res = stmt->Execute({Quote(phone), Quote(name)});

                        if (!res->Read())
                            throw std::runtime_error{"Failed to put phone number \"" + phone + "\" for name \"" + name + "\""};

                        tr.Commit();
                    }

                    virtual std::map<std::string, std::string> Find(std::string const &name) const override final
                    {
                        auto conn = m_connections->GetConnection();
                        Mif::Db::Transaction tr{conn};
                        Mif::Common::Unused(tr);

                        auto stmt = conn->CreateStatement(
                                "select phone, name "
                                "from phones "
                                "where name like $1::text "
                                "order by name;"
                            );
                        auto res = stmt->Execute({Quote("%" + name + "%")});

                        std::map<std::string, std::string> phones;

                        while (res->Read())
                        {
                            std::size_t index = 0;

                            auto phone = res->GetAsString(index++);
                            auto name = res->GetAsString(index++);

                            phones.emplace(std::move(phone), std::move(name));
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
    ::Phonebook::Id::PgStorage,
    ::Phonebook::Service::Detail::Storage,
    Mif::Application::IConfigPtr
)
