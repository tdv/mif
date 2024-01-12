//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     04.2021
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

// STD
#include <sstream>

// BOOST
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/uuid/name_generator_sha1.hpp>
#include <boost/uuid/uuid_io.hpp>


// MIF
#include "mif/common/crc32.h"
#include "mif/common/hash_calc.h"

namespace Mif
{
    namespace Common
    {
        namespace Hash
        {

            std::string Crc32(void const *data, std::size_t size)
            {
                return std::to_string(Common::Crc32(data, size));
            }

            std::string Md5(void const *data, std::size_t size)
            {
                if (!data || !size)
                {
                    throw std::invalid_argument{"Failed to calc md5 for empty data."};
                }

                boost::uuids::detail::md5 hash;
                boost::uuids::detail::md5::digest_type digest;
                hash.process_bytes(data, size);
                hash.get_digest(digest);

                auto const charDigest = reinterpret_cast<char const *>(&digest);
                std::string result;
                boost::algorithm::hex(charDigest, charDigest + sizeof(boost::uuids::detail::md5::digest_type),
                        std::back_inserter(result));
                boost::algorithm::to_lower(result);

                return result;
            }

            std::string Sha1(void const *data, std::size_t size)
            {
                if (!data || !size)
                {
                    throw std::invalid_argument{"Failed to calc sha1 for empty data."};
                }

                boost::uuids::name_generator_sha1 gen{boost::uuids::ns::dns()};
                std::stringstream stresm;
                stresm << gen(std::string{reinterpret_cast<char const *>(data), size});
                return stresm.str();
            }

        }   // namespace Hash
    }   // namespace Common
}   // namespace Mif
