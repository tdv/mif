//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_ORM_STORAGE_H__
#define __MIF_ORM_STORAGE_H__

// STD
#include <stdexcept>
#include <type_traits>

// MIF
#include "mif/db/iconnection.h"
#include "mif/orm/schema.h"
#include "mif/reflection/reflection.h"
#include "mif/serialization/traits.h"

namespace Mif
{
    namespace Orm
    {

        template <typename TSchema, template <typename ... > class TSqlDriver>
        class Storage final
        {
        public:
            Storage(Db::IConnectionPtr connection)
                : m_connection{connection}
            {
                if (!m_connection)
                    throw std::invalid_argument{"[Mif::Orm::Storage] Empty connection ptr."};
            }

            void Create()
            {
                auto const sql = SqlDriver::CreateSchema();
                m_connection->ExecuteDirect(sql);
            }

            template <typename T>
            typename std::enable_if
            <
                Reflection::IsReflectable<T>() &&
                !Serialization::Traits::IsIterable<T>() &&
                // TODO: check the including into schema
                true,
                void
            >::type
            Put(T const &value)
            {
                // TODO: put value
            }

        private:
            using SqlDriver = TSqlDriver<TSchema>;
            Db::IConnectionPtr m_connection;
        };

    }   // namespace Orm
}   // namespace Mif

#endif  // !__MIF_ORM_STORAGE_H__
