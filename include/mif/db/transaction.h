//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_DB_TRANSACTION_H__
#define __MIF_DB_TRANSACTION_H__

// MIF
#include "mif/db/iconnection.h"

namespace Mif
{
    namespace Db
    {

        class Transaction final
        {
        public:
            enum class State
            {
                Unknown,
                Init,
                Begin,
                Commit,
                Rollback
            };

            Transaction(Transaction const &) = delete;
            Transaction& operator = (Transaction const &) = delete;

            Transaction(IConnectionPtr connection, bool open = true);
            Transaction(Transaction &&transaction);
            Transaction& operator = (Transaction &&transaction);

            ~Transaction();

            void Begin();
            void Commit();
            void Rollback();
            State GetState() const;

        private:
            IConnectionPtr m_connection;
            State m_state = State::Init;
        };

    }   // namespace Db
}   // namespace Mif

#endif  // !__MIF_DB_TRANSACTION_H__
