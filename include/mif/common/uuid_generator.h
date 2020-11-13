//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_COMMON_UUID_GENERATOR_H__
#define __MIF_COMMON_UUID_GENERATOR_H__

// STD
#include <memory>
#include <string>

namespace Mif
{
    namespace Common
    {

        class UuidGenerator final
        {
        public:
            UuidGenerator();
            ~UuidGenerator();

            std::string Generate() const;

        private:
            class Impl;
            std::unique_ptr<Impl> m_impl;
        };

    }   // namespace Common
}   // namespace Mif

#endif  // !__MIF_COMMON_UUID_GENERATOR_H__
