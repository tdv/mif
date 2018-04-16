// MIF
#include <mif/service/creator.h>

// THIS
#include "base.h"
#include "data/meta/api.h"
#include "id/service.h"
#include "interface/iadmin.h"

namespace CacheService
{
    namespace Handler
    {
        namespace Detail
        {
            namespace
            {

                class Admin
                    : public Base
                {
                public:
                    Admin(std::string const &prefix)
                        : Base{prefix}
                    {
                        AddHandler("/profile/list", this, &Admin::List);
                        AddHandler("/profile/get", this, &Admin::Get);
                        AddHandler("/profile/set", this, &Admin::Set);
                        AddHandler("/profile/remove", this, &Admin::Remove);
                    }

                private:
                    // WebService.Hadlers
                    Response List()
                    {
                        return {std::string{}};
                    }

                    Response Get()
                    {
                        return {std::string{}};
                    }

                    Response Set()
                    {
                        return {std::string{}};
                    }

                    Response Remove()
                    {
                        return {std::string{}};
                    }
                };

            }   // namespace
        }   // namespace Detail
    }   // namespace Handler
}   // namespace CacheService

MIF_SERVICE_CREATOR
(
    ::CacheService::Id::Admin,
    ::CacheService::Handler::Detail::Admin,
    std::string
)
