// STD
#include <mutex>

// MIF
#include <mif/application/iconfig.h>
#include <mif/service/creator.h>

// THIS
#include "exception.h"
#include "id/service.h"
#include "interface/istorage.h"

namespace Phonebook
{
    namespace Service
    {
        namespace Detail
        {
            namespace
            {

                class Storage
                    : public Mif::Service::Inherit<IStorage>
                {
                public:
                    Storage(Mif::Application::IConfigPtr config)
                    {
                        if (!config)
                            throw std::invalid_argument{"Storage. No config."};
                    }

                private:
                    using LockType = std::mutex;
                    using LockGuard = std::lock_guard<LockType>;

                    // IStorage
                };

            }   // namespace
        }   // namespace Detail
    }   // namespace Service
}   // namespace Phonebook

MIF_SERVICE_CREATOR
(
    ::Phonebook::Id::FileStorage,
    ::Phonebook::Service::Detail::Storage,
    Mif::Application::IConfigPtr
)
