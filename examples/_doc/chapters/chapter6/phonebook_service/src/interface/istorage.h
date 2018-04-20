#ifndef __PHONEBOOK_INTERFACE_ISTORAGE_H__
#define __PHONEBOOK_INTERFACE_ISTORAGE_H__

// STD
#include <map>
#include <string>

// MIF
#include <mif/service/iservice.h>

namespace Phonebook
{
    namespace Service
    {

        struct IStorage
            : public Mif::Service::Inherit<Mif::Service::IService>
        {
            virtual void Set(std::string const &phone, std::string const &name) = 0;
            virtual std::map<std::string, std::string> Find(std::string const &name) const = 0;
            virtual std::map<std::string, std::string> List() const = 0;
        };

        using IStoragePtr = Mif::Service::TServicePtr<IStorage>;

    }   // namespace Service
}   // namespace Phonebook

#endif  // !__PHONEBOOK_INTERFACE_ISTORAGE_H__
