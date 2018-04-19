// BOOST
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// MIF
#include <mif/common/log.h>
#include <mif/common/unused.h>
#include <mif/net/http/constants.h>
#include <mif/serialization/json.h>
#include <mif/service/root_locator.h>

// THIS
#include "base.h"
#include "data/meta/api.h"
#include "exception.h"
#include "id/service.h"
#include "interface/iauth.h"
#include "version/version.h"

namespace CacheService
{
    namespace Handler
    {

        Base::Base(std::string const &prefix)
            : m_prefix{prefix}
        {
        }

        Data::Api::Response::Info Base::GetMeta(std::int32_t code, std::string message) const
        {
            Data::Api::Response::Info info;

            info.service = Version::GetFullName();
            info.node = boost::asio::ip::host_name();
            auto const now = boost::posix_time::second_clock::universal_time();
            info.timestamp = boost::posix_time::to_iso_string(now);
            info.status.code = code;
            info.status.message = message;

            return info;
        }

        void Base::OnException(Mif::Net::Http::IInputPack const &request,
                               Mif::Net::Http::IOutputPack &response, std::exception_ptr exception,
                               Mif::Net::Http::Code &code, std::string &message) const
        {
            Mif::Common::Unused(request);

            try
            {
                MIF_LOG(Warning) << "[Service::Base] Failed to process request. Error: " << message;

                Data::Api::Response::Header data;
                data.meta = GetMeta(-1, message);

                try
                {
                    std::rethrow_exception(exception);
                }
                catch (Exception::Unauthorized &e)
                {
                    data.meta.status.message = e.what();
                    code = Mif::Net::Http::Code::Unauthorized;
                }
                catch (Exception::NotFound &e)
                {
                    data.meta.status.message = e.what();
                    code = Mif::Net::Http::Code::NotFound;
                }
                catch (...)
                {
                }

                auto const buffer = Mif::Serialization::Json::Serialize(data);
                message = {std::begin(buffer), std::end(buffer)};

                response.SetCode(code);
                response.SetHeader(Mif::Net::Http::Constants::Header::Response::ContentType::Value,
                        Mif::Net::Http::Serializer::Json::GetContentType());
            }
            catch (std::exception const &e)
            {
                MIF_LOG(Error) << "[Service::Base] Failed to format message. Error: " << e.what() << " Message: " << message;
            }
        }

        Data::ID Base::GetSession(Headers const &headers) const
        {
            std::string session;

            for (auto const &header : headers.Get())
            {
                std::string const cookeiId = Mif::Net::Http::Constants::Header::Request::Cookie::Value;
                std::string cookieKey;
                std::transform(std::begin(cookeiId), std::end(cookeiId),
                               std::inserter(cookieKey, std::end(cookieKey)), [] (std::string::value_type ch) { return std::tolower(ch); } );

                std::string headerKey;
                std::transform(std::begin(header.first), std::end(header.first),
                               std::inserter(headerKey, std::end(headerKey)), [] (std::string::value_type ch) { return std::tolower(ch); } );

                if (cookieKey == headerKey)
                {
                    Data::IDs items;
                    boost::algorithm::split(items, header.second, boost::is_any_of(";"));
                    if (!items.empty())
                    {
                        for (auto const &item : items)
                        {
                            std::vector<std::string> cookie;
                            boost::algorithm::split(cookie, item, boost::is_any_of("="));
                            for (auto &i : cookie)
                                boost::algorithm::trim(i);

                            if (cookie.size() >= 2 && cookie.front() == CookieId::Value)
                            {
                                session = cookie[1];
                                break;
                            }
                        }
                    }
                    break;
                }
            }

            return session;
        }

        Data::Profile Base::GetProfile(Headers const &headers) const
        {
            auto const session = GetSession(headers);
            if (session.empty())
                throw Exception::Unauthorized{"No session."};

            auto auth = Mif::Service::RootLocator::Get()->Get<IAuth>(Id::DataFacade);
            return auth->GetSessionProfile(session);
        }

        Data::Profile Base::CheckPermissions(Headers const &headers, Data::Roles const &roles) const
        {
            auto const profile = GetProfile(headers);

            Data::Roles result;

            std::set_intersection(std::begin(profile.roles), std::end(profile.roles),
                    std::begin(roles), std::end(roles),
                    std::inserter(result, std::begin(result))
                );

            if (result.empty())
            {
                throw std::runtime_error{"Access denied."};
            }

            return profile;
        }

    }   // namespace Handler
}   // namespace CacheService
