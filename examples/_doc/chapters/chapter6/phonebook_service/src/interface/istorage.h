#ifndef __PHONEBOOK_INTERFACE_ISTORAGE_H__
#define __PHONEBOOK_INTERFACE_ISTORAGE_H__

// MIF
#include <mif/service/iservice.h>

// THIS
#include "data/data.h"

namespace Phonebook
{
    namespace Service
    {

        struct IStorage
            : public Mif::Service::Inherit<Mif::Service::IService>
        {
        };

        using IStoragePtr = Mif::Service::TServicePtr<IStorage>;

    }   // namespace Service
}   // namespace Phonebook

#endif  // !__PHONEBOOK_INTERFACE_ISTORAGE_H__
