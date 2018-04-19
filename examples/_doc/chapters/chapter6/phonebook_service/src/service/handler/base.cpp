// BOOST
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// MIF
#include <mif/common/log.h>
#include <mif/common/unused.h>
#include <mif/net/http/constants.h>
#include <mif/serialization/json.h>

// THIS
#include "base.h"
#include "data/meta/data.h"
#include "exception.h"
#include "id/service.h"
#include "version/version.h"

namespace Phonebook
{
    namespace Service
    {
        namespace Handler
        {

            Base::Base(std::string const &prefix)
                : m_prefix{prefix}
            {
            }

            Data::Info Base::GetMeta(std::int32_t code, std::string message) const
            {
                Data::Info info;

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

                    Data::Header data;
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

        }   // namespace Handler
    }   // namespace Service
}   // namespace Phonebook
