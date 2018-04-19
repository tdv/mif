// STD
#include <clocale>
#include <sstream>

// BOOST
#include <boost/date_time/posix_time/posix_time.hpp>

// MIF
#include <mif/net/http/constants.h>
#include <mif/service/creator.h>
#include <mif/service/root_locator.h>

// THIS
#include "base.h"
#include "data/meta/api.h"
#include "id/service.h"
#include "interface/iauth.h"

namespace CacheService
{
    namespace Handler
    {
        namespace Detail
        {
            namespace
            {

                class Auth
                    : public Base
                {
                public:
                    Auth(std::string const &prefix)
                        : Base{prefix}
                    {
                        AddHandler("/login", this, &Auth::Login);
                        AddHandler("/logout", this, &Auth::Logout);
                    }

                private:
                    // WebService.Hadlers
                    Response Login(Prm<std::string, Name("login")> const &login,
                                   Prm<std::string, Name("password")> const &password)
                    {
                        if (!login)
                            throw std::invalid_argument{"No \"login\" parameter."};
                        if (login.Get().empty())
                            throw std::invalid_argument{"\"login\" must not be empty."};

                        if (!password)
                            throw std::invalid_argument{"No \"password\" parameter."};
                        if (password.Get().empty())
                            throw std::invalid_argument{"\"password\" must not be empty."};

                        auto auth = Mif::Service::RootLocator::Get()->Get<IAuth>(Id::DataFacade);

                        auto const session = auth->Login(login.Get(), password.Get());

                        auto tsFormatt = [] (boost::posix_time::ptime const &timestamp)
                            {
                                auto const *format = "%a, %d %b %Y %H:%M:%S GMT";
                                std::ostringstream stream;
                                stream.imbue(std::locale(std::locale::classic(), new boost::posix_time::time_facet(format)));
                                stream << timestamp;
                                return stream.str();
                            };

                        auto const cookie = std::string{CookieId::Value} + "=" + session.id +
                                "; Path=/; Expires=" + tsFormatt(session.expires) + ";";

                        Data::Api::Response::Id response;

                        response.meta = GetMeta();
                        response.data = session.id;

                        return {response, {{Mif::Net::Http::Constants::Header::Response::SetCookie::Value, cookie}}};
                    }

                    Response Logout(Headers const &headers)
                    {
                        auto session = GetSession(headers);

                        auto auth = Mif::Service::RootLocator::Get()->Get<IAuth>(Id::DataFacade);
                        auth->Logout(session);

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
    ::CacheService::Id::Auth,
    ::CacheService::Handler::Detail::Auth,
    std::string
)
