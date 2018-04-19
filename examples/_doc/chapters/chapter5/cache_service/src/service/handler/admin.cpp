// MIF
#include <mif/service/creator.h>
#include <mif/service/root_locator.h>

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
                    Response List(Headers const &headers)
                    {
                        CheckPermissions(headers, {Data::Role::Admin});

                        auto admin = Mif::Service::RootLocator::Get()->Get<IAdmin>(Id::DataFacade);

                        Data::Api::Response::ProfilesList response;

                        response.meta = GetMeta();
                        response.data = admin->GetProfilesList();

                        return response;
                    }

                    Response Get(Headers const &headers, Prm<Data::ID, Name("login")> const &login)
                    {
                        CheckPermissions(headers, {Data::Role::Admin});

                        if (!login)
                            throw std::invalid_argument{"No \"login\" parameter."};

                        if (login.Get().empty())
                            throw std::invalid_argument{"\"login\" must not be empty."};

                        auto admin = Mif::Service::RootLocator::Get()->Get<IAdmin>(Id::DataFacade);

                        Data::Api::Response::Profile response;

                        response.meta = GetMeta();
                        response.data = admin->GetProfile(login.Get());

                        return response;
                    }

                    Response Set(Headers const &headers, RequestContent<Data::Profile> const &content)
                    {
                        CheckPermissions(headers, {Data::Role::Admin});

                        auto const &profile = content.Get();

                        if (profile.login.empty())
                            throw std::invalid_argument{"\"login\" must not be empty."};
                        if (profile.password.empty())
                            throw std::invalid_argument{"\"password\" must not be empty."};

                        auto admin = Mif::Service::RootLocator::Get()->Get<IAdmin>(Id::DataFacade);
                        admin->SetProfile(profile);

                        Data::Api::Response::Header response;

                        response.meta = GetMeta();

                        return response;
                    }

                    Response Remove(Headers const &headers, Prm<Data::ID, Name("login")> const &login)
                    {
                        CheckPermissions(headers, {Data::Role::Admin});

                        if (!login)
                            throw std::invalid_argument{"No \"login\" parameter."};
                        if (login.Get().empty())
                            throw std::invalid_argument{"\"login\" must not be empty."};

                        auto admin = Mif::Service::RootLocator::Get()->Get<IAdmin>(Id::DataFacade);
                        admin->RemoveProfile(login.Get());

                        Data::Api::Response::Header response;

                        response.meta = GetMeta();

                        return response;
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
