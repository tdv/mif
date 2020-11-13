//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

// STD
#include <mutex>
#include <sstream>

// BOOST
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

// MIF
#include "mif/common/uuid_generator.h"

namespace Mif
{
    namespace Common
    {

        class UuidGenerator::Impl final
        {
        public:
            std::string Generate() const
            {
                std::stringstream stream;
                {
                    std::lock_guard<std::mutex> lock(m_lock);
                    auto const uuid = m_generator();
                    stream << uuid;
                }
                return stream.str();
            }

        private:
            mutable std::mutex m_lock;
            mutable boost::uuids::random_generator m_generator;
        };


        UuidGenerator::UuidGenerator()
            : m_impl{new Impl}
        {
        }

        UuidGenerator::~UuidGenerator()
        {
        }

        std::string UuidGenerator::Generate() const
        {
            return m_impl->Generate();
        }

    }   // namespace Common
}   // namespace Mif
