//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MICROSERVICES_SERVICE_DATA_META_RESPONSES_H__
#define __MICROSERVICES_SERVICE_DATA_META_RESPONSES_H__

// MIF
#include <mif/reflection/reflect_type.h>

// COMMON
#include "common/data/meta/data.h"

// THIS
#include "../responses.h"

namespace Service
{
    namespace Data
    {
        namespace Response
        {
            namespace Meta
            {

                MIF_REFLECT_BEGIN(Status)
                    MIF_REFLECT_FIELD(code)
                    MIF_REFLECT_FIELD(message)
                MIF_REFLECT_END()

                MIF_REFLECT_BEGIN(Info)
                    MIF_REFLECT_FIELD(timestamp)
                    MIF_REFLECT_FIELD(node)
                    MIF_REFLECT_FIELD(status)
                MIF_REFLECT_END()

                MIF_REFLECT_BEGIN(Void)
                    MIF_REFLECT_FIELD(meta)
                MIF_REFLECT_END()

                MIF_REFLECT_BEGIN(Id)
                    MIF_REFLECT_FIELD(meta)
                    MIF_REFLECT_FIELD(data)
                MIF_REFLECT_END()

                MIF_REFLECT_BEGIN(Employee)
                    MIF_REFLECT_FIELD(meta)
                    MIF_REFLECT_FIELD(data)
                MIF_REFLECT_END()

                MIF_REFLECT_BEGIN(Employees)
                    MIF_REFLECT_FIELD(meta)
                    MIF_REFLECT_FIELD(data)
                MIF_REFLECT_END()

            }   // namespace Meta
        }   // namespace Response
    }   // namespace Data
}   // namespace Service

MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Response::Meta::Status)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Response::Meta::Info)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Response::Meta::Void)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Response::Meta::Id)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Response::Meta::Employee)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Response::Meta::Employees)

#endif  // !__MICROSERVICES_SERVICE_DATA_META_RESPONSES_H__
