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
#include <typeinfo>
#include <utility>

// BOOST
#include <boost/intrusive_ptr.hpp>

#define MIF_ISERVICE_METHODS_DECL_IMPL() \
    public: \
        virtual std::size_t AddRef() = 0; \
        virtual std::size_t Release() = 0;  \
    private: \
        virtual bool Query(std::type_info const &typeInfo, \
                void **service, std::string const &serviceId = {}) = 0; \
    public: \
        template <typename T_Mif_Based_Interface__> \
        TServicePtr<T_Mif_Based_Interface__> Query(std::string const &serviceId = {}) \
        { \
            T_Mif_Based_Interface__ *instance = nullptr; \
            auto const result = Query(typeid(T_Mif_Based_Interface__), reinterpret_cast<void **>(&instance), serviceId); \
            if (!result || !instance) \
                return {}; \
            TServicePtr<T_Mif_Based_Interface__> service{instance}; \
            /*service->Release();*/ \
            return service; \
        } \
        template <typename T_Mif_Based_Interface__> \
        TServicePtr<T_Mif_Based_Interface__> Cast(std::string const &serviceId = {}) \
        { \
            if (auto p = this->template Query<T_Mif_Based_Interface__>(serviceId)) \
                return p; \
            throw std::runtime_error{"[Mif::Service::Cast] Failed to cast interface."}; \
        } \
    private: \
        template <typename> \
        friend class Detail::Service_Impl__; \
        template <typename T_Mif_Based_Interface__> \
        bool QueryInterfaceInternal(void **service, T_Mif_Based_Interface__ *instance, \
                std::type_info const &typeInfo, std::string const &serviceId) \
        { \
            return Detail::QueryInterface<TBaseTypeTuple>::Query(service, static_cast<TThisInterfaceItemType *>(instance), \
                    typeInfo, serviceId); \
        }

#define MIF_SERVICE_ID(id_) \
    \

namespace Mif
{
    namespace Service
    {

        struct IService;

        namespace Detail
        {

            template <typename T>
            class Service_Impl__;

            struct IProxyBase_Mif_Remote_
            {
                virtual ~IProxyBase_Mif_Remote_() = default;
                virtual bool _Mif_Remote_QueryRemoteInterface(void **service,
                        std::type_info const &typeInfo, std::string const &serviceId) = 0;
            };

            template <typename>
            struct QueryInterface;

            template <typename H, typename ... T>
            struct QueryInterface<std::tuple<H, T ... >>
            {
                static bool Query(void **service, void *instance, std::type_info const &typeInfo,
                        std::string const &serviceId)
                {
                    if (typeid(H) == typeInfo)
                    {
                        auto obj = static_cast<H *>(instance);
                        /*obj->AddRef();*/
                        *service = obj;
                        return true;
                    }
                    using TBases = typename H::TThisInterfaceItemType::TBaseTypeTuple;
                    if (QueryInterface<TBases>::Query(service, instance, typeInfo, serviceId))
                        return true;
                    return QueryInterface<std::tuple<T ... >>::Query(service, instance, typeInfo, serviceId);
                }
            };

            template <>
            struct QueryInterface<std::tuple<>>
            {
                static bool Query(void **, void *, std::type_info const &, std::string const &)
                {
                    return false;
                }
            };

        }   // namespace Detail

        using ServiceId = std::uint32_t;

        template <typename T>
        using TIntrusivePtr = boost::intrusive_ptr<T>;

        template <typename T>
        using TServicePtr = typename std::enable_if
            <
                std::is_base_of<IService, T>::value || std::is_same<IService, T>::value,
                TIntrusivePtr<T>
            >::type;

        using IServicePtr = TServicePtr<IService>;

        struct IService
        {
            using TThisInterfaceItemType = IService;
            using TBaseTypeTuple = std::tuple<>;

            MIF_SERVICE_ID("Mif.Service.IService")

            virtual ~IService() = default;

            MIF_ISERVICE_METHODS_DECL_IMPL()

        private:
            template <typename T>
            friend class Detail::Service_Impl__;

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

        template <typename ... T>
        struct Inherit
            : public std::enable_if<std::is_base_of<IService, T>::value || std::is_same<IService, T>::value, T>::type ...
        {
            using TThisInterfaceItemType = Inherit<T ... >;
            using TBaseTypeTuple = std::tuple<T ... >;

            virtual ~Inherit() = default;

            MIF_ISERVICE_METHODS_DECL_IMPL()
        };

        template <typename T>
        inline typename std::enable_if<std::is_base_of<IService, T>::value || std::is_same<IService, T>::value, TServicePtr<T>>::type
        Query(IServicePtr service, std::string const &serviceId = {})
        {
            if (!service)
                throw std::invalid_argument{"[Mif::Service::Query] You can't query interface from empty pointer."};

            TServicePtr<T> res{dynamic_cast<T *>(service.get())};
            return res ? res : service->Query<T>(serviceId);
        }

        template <typename T>
        inline typename std::enable_if<std::is_base_of<IService, T>::value || std::is_same<IService, T>::value, TServicePtr<T>>::type
        Cast(IServicePtr service, std::string const &serviceId = {})
        {
            if (auto p = Query<T>(service, serviceId))
                return p;

            throw std::runtime_error{"[Mif::Service::Cast] Failed to cast interface."};
        }

    }  // namespace Service
}   // namespace Mif

#endif  // !__MIF_SERVICE_ISERVICE_H__
