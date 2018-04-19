#ifndef __PHONEBOOK_ID_SERVICE_H__
#define __PHONEBOOK_ID_SERVICE_H__

// MIF
#include <mif/common/crc32.h>

namespace Phonebook
{
    namespace Id
    {

        enum
        {
            Storage = Mif::Common::Crc32("Phonebook.Id.Storage"),
            PgStorage = Mif::Common::Crc32("Phonebook.Id.PgStorage"),
            FileStorage = Mif::Common::Crc32("Phonebook.Id.FileStorage"),
            Phonebook = Mif::Common::Crc32("Phonebook.Id.Phonebook")
        };

    }   // namespace Id
}   // namespace Phonebook

#endif  // !__PHONEBOOK_ID_SERVICE_H__
