//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// MIF
#include "mif/common/log.h"
#include "mif/db/transaction.h"

namespace Mif
{
    namespace Db
    {

        Transaction::Transaction(IConnectionPtr connection, bool open)
            : m_connection{connection}
        {
            if (!m_connection)
                throw std::invalid_argument{"[Mif::Db::Transaction] Empty transaction pointer."};

            if (open)
                Begin();
        }

        Transaction::Transaction(Transaction &&transaction)
            : m_connection{transaction.m_connection}
            , m_state(transaction.m_state)
        {
            transaction.m_connection.reset();
            transaction.m_state = State::Unknown;
        }

        Transaction& Transaction::operator = (Transaction &&transaction)
        {
            m_connection = transaction.m_connection;
            m_state = transaction.m_state;

            transaction.m_connection.reset();
            transaction.m_state = State::Unknown;

            return *this;
        }

        Transaction::~Transaction()
        {
            if (!m_connection || m_state != State::Begin)
                return;

            try
            {
                Rollback();
            }
            catch (std::exception const &e)
            {
                MIF_LOG(Warning) << "[Mif::Db::~Transaction] Failed to rollback transaction. "
                        << "Error: " << e.what();
            }
        }

        void Transaction::Begin()
        {
            if (!m_connection)
                throw std::logic_error{"[Mif::Db::Transaction::Begin] Empty transaction pointer."};

            if (m_state != State::Init)
                throw std::logic_error{"[Mif::Db::Transaction::Begin] Bad transaction state."};

            try
            {
                m_connection->ExecuteDirect("BEGIN;");
            }
            catch (std::exception const &e)
            {
                throw std::logic_error{"[Mif::Db::Transaction::Begin] "
                        "Failed to start transaction. Error: " + std::string{e.what()}};
            }

            m_state = State::Begin;
        }

        void Transaction::Commit()
        {
            if (!m_connection)
                throw std::logic_error{"[Mif::Db::Transaction::Commit] Empty transaction pointer."};

            if (m_state != State::Begin)
                throw std::logic_error{"[Mif::Db::Transaction::Commit] Bad transaction state."};

            try
            {
                m_connection->ExecuteDirect("COMMIT;");
            }
            catch (std::exception const &e)
            {
                throw std::logic_error{"[Mif::Db::Transaction::Commit] "
                        "Failed to commit transaction. Error: " + std::string{e.what()}};
            }

            m_state = State::Commit;
        }

        void Transaction::Rollback()
        {
            if (!m_connection)
                throw std::logic_error{"[Mif::Db::Transaction::Rollback] Empty transaction pointer."};

            if (m_state != State::Begin)
                throw std::logic_error{"[Mif::Db::Transaction::Rollback] Bad transaction state."};

            try
            {
                m_connection->ExecuteDirect("ROLLBACK;");
            }
            catch (std::exception const &e)
            {
                throw std::logic_error{"[Mif::Db::Transaction::Rollback] "
                        "Failed to rollback transaction. Error: " + std::string{e.what()}};
            }

            m_state = State::Rollback;
        }

        Transaction::State Transaction::GetState() const
        {
            return m_state;
        }

    }   // namespace Db
}   // namespace Mif
