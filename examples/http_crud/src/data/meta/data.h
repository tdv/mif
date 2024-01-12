//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     07.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __HTTP_CRUD_DATA_META_DATA_H__
#define __HTTP_CRUD_DATA_META_DATA_H__

// MIF
#include <mif/reflection/reflect_type.h>

// THIS
#include "data/data.h"

namespace Service
{
    namespace Data
    {
        namespace Meta
        {

            MIF_REFLECT_BEGIN(Position)
                MIF_REFLECT_FIELD(Unknown)
                MIF_REFLECT_FIELD(Developer)
                MIF_REFLECT_FIELD(Manager)
            MIF_REFLECT_END()

            MIF_REFLECT_BEGIN(Employee)
                MIF_REFLECT_FIELD(name)
                MIF_REFLECT_FIELD(lastName)
                MIF_REFLECT_FIELD(age)
                MIF_REFLECT_FIELD(email)
                MIF_REFLECT_FIELD(position)
                MIF_REFLECT_FIELD(rate)
            MIF_REFLECT_END()

        }   // namespace Meta

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

MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Meta::Position)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Meta::Employee)

MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Response::Meta::Status)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Response::Meta::Info)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Response::Meta::Void)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Response::Meta::Id)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Response::Meta::Employee)
MIF_REGISTER_REFLECTED_TYPE(::Service::Data::Response::Meta::Employees)

#endif  // !__HTTP_CRUD_DATA_META_DATA_H__
