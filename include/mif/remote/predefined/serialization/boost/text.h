//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_PREDEFINED_SERIALIZATION_BOOST_TEXT_H__
#define __MIF_REMOTE_PREDEFINED_SERIALIZATION_BOOST_TEXT_H__

// BOOST
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

// MIF

#include "mif/remote/serialization/serialization.h"
#include "mif/remote/serialization/boost.h"

namespace Mif
{
    namespace Remote
    {
        namespace Predefined
        {
            namespace Serialization
            {
                namespace Boost
                {

                    using Text = Remote::Serialization::SerializerTraits
                            <
                                Remote::Serialization::Boost::Serializer<boost::archive::text_oarchive>,
                                Remote::Serialization::Boost::Deserializer<boost::archive::text_iarchive>
                            >;

                }   // namespace Boost
            }   // namespace Serialization
        }   // namespace Predefined
    }   // namespace Remote
}   // namespace Mif

#endif  // !__MIF_REMOTE_PREDEFINED_SERIALIZATION_BOOST_TEXT_H__
