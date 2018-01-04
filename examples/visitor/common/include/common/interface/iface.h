//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __VISITOR_COMMON_INTERFACE_IFACE_H__
#define __VISITOR_COMMON_INTERFACE_IFACE_H__

// STD
#include <string>

// MIF
#include <mif/service/iservice.h>

namespace Service
{

    struct IMessage
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
        virtual std::string GetText() const = 0;
    };

    using IMessagePtr = Mif::Service::TIntrusivePtr<IMessage>;

    struct IMessageVisitor
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
        virtual void Visit(IMessagePtr message) = 0 ;
    };

    using IMessageVisitorPtr = Mif::Service::TIntrusivePtr<IMessageVisitor>;

    struct IViewer
        : public Mif::Service::Inherit<Mif::Service::IService>
    {
        virtual void Accept(IMessageVisitorPtr visitor) = 0;
    };

}   // namespace Service

#endif  // !__VISITOR_COMMON_INTERFACE_IFACE_H__
