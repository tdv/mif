//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_ISERVICE_H__
#define __MIF_SERVICE_ISERVICE_H__

// STD
#include <cstdint>
#include <stdexcept>
#include <tuple>
#include <utility>

// BOOST
#include <boost/intrusive_ptr.hpp>

#define MIF_ISERVICE_METHODS_DECL_IMPL() \
    virtual std::size_t AddRef() = 0; \
    virtual std::size_t Release() = 0;

#define MIF_SERVICE_ID(id_) \
    \

namespace Mif
{
    namespace Service
    {

        using ServiceId = std::uint32_t;

        struct IService
        {
            MIF_SERVICE_ID("Mif.Service.IService")

            virtual ~IService() = default;

            MIF_ISERVICE_METHODS_DECL_IMPL()

        private:
            template<typename T>
            friend typename std::enable_if<std::is_base_of<IService, T>::value || std::is_same<IService, T>::value, void>::type
            intrusive_ptr_add_ref(T *ptr)
            {
                if (ptr)
                    ptr->AddRef();
            }

            template<typename T>
            friend typename std::enable_if<std::is_base_of<IService, T>::value || std::is_same<IService, T>::value, void>::type
            intrusive_ptr_release(T *ptr)
            {
                if (ptr)
                    ptr->Release();
            }

        };

        template <typename T>
        using TIntrusivePtr = boost::intrusive_ptr<T>;

        template <typename T>
        using TServicePtr = typename std::enable_if
            <
                std::is_base_of<IService, T>::value || std::is_same<IService, T>::value,
                TIntrusivePtr<T>
            >::type;

        using IServicePtr = TServicePtr<IService>;

        template <typename ... T>
        struct Inherit
            : public std::enable_if<std::is_base_of<IService, T>::value || std::is_same<IService, T>::value, T>::type ...
        {
            using TBaseTypeTuple = std::tuple<T ... >;

            virtual ~Inherit() = default;

            MIF_ISERVICE_METHODS_DECL_IMPL()
        };

        template <typename T>
        inline typename std::enable_if<std::is_base_of<IService, T>::value || std::is_same<IService, T>::value, TServicePtr<T>>::type
        Query(IServicePtr service)
        {
            if (!service)
                throw std::invalid_argument{"[Mif::Service::Query] You can't query interface from empty pointer."};

            return TServicePtr<T>{dynamic_cast<T *>(service.get())};
        }

        template <typename T>
        inline typename std::enable_if<std::is_base_of<IService, T>::value || std::is_same<IService, T>::value, TServicePtr<T>>::type
        Cast(IServicePtr service)
        {
            if (auto p = Query<T>(service))
                return p;

            throw std::runtime_error{"[Mif::Service::Cast] Failed to cast interface."};
        }

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_ISERVICE_H__
