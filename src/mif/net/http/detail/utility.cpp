//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

// STD
#include <regex>
#include <sstream>
#include <stdexcept>

// BOOST
#include <boost/date_time/posix_time/posix_time.hpp>

// THIS
#include "utility.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {
                namespace Utility
                {

                    Method::Type ConvertMethodType(boost::beast::http::verb verb)
                    {
                        switch (verb)
                        {
                        case boost::beast::http::verb::get :
                            return Method::Type::Get;
                        case boost::beast::http::verb::post :
                            return Method::Type::Post;
                        case boost::beast::http::verb::head :
                            return Method::Type::Head;
                        case boost::beast::http::verb::put :
                            return Method::Type::Put;
                        case boost::beast::http::verb::delete_ :
                            return Method::Type::Delete;
                        case boost::beast::http::verb::options :
                            return Method::Type::Options;
                        case boost::beast::http::verb::trace :
                            return Method::Type::Trqce;
                        case boost::beast::http::verb::connect :
                            return Method::Type::Connect;
                        case boost::beast::http::verb::patch :
                            return Method::Type::Patch;
                        default :
                            break;
                        }

                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Utility::ConvertMethodType] "
                            "Unknown method type."};
                    }

                    boost::beast::http::verb ConvertMethodType(Method::Type method)
                    {
                        switch (method)
                        {
                        case Method::Type::Get :
                            return boost::beast::http::verb::get ;
                        case Method::Type::Post :
                            return boost::beast::http::verb::post ;
                        case Method::Type::Head :
                            return boost::beast::http::verb::head ;
                        case Method::Type::Put :
                            return boost::beast::http::verb::put ;
                        case Method::Type::Delete :
                            return boost::beast::http::verb::delete_ ;
                        case Method::Type::Options :
                            return boost::beast::http::verb::options ;
                        case Method::Type::Trqce :
                            return boost::beast::http::verb::trace ;
                        case Method::Type::Connect :
                            return boost::beast::http::verb::connect ;
                        case Method::Type::Patch :
                            return boost::beast::http::verb::patch ;
                        default :
                            break;
                        }

                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Utility::ConvertMethodType] "
                            "Unknown method type."};
                    }

                    char const* GetReasonString(Code code)
                    {
                        switch (code)
                        {
                        case Code::Ok :
                            return "OK";
                        case Code::NoContent :
                            return "Request does not have content";
                        case Code::MovePerm :
                            return "The uri moved permanently";
                        case Code::MoveTemp :
                            return "The uri moved temporarily";
                        case Code::NotModified :
                            return "Page was not modified from last";
                        case Code::BadRequest :
                            return "Invalid http request was made";
                        case Code::NotFound :
                            return "Could not find content for uri";
                        case Code::Unauthorized :
                            return "Unauthorized";
                        case Code::BadMethod :
                            return "Method not allowed for this uri";
                        case Code::Internal :
                            return "Internal error";
                        case Code::NotImplemented :
                            return "Not implemented";
                        case Code::Unavaliable :
                            return "The server is not available";
                        default :
                            break;
                        }

                        return "";
                    }

                    boost::beast::http::status ConvertCode(Code code)
                    {
                        switch (code)
                        {
                        case Code::Ok :
                            return boost::beast::http::status::ok;
                        case Code::NoContent :
                            return boost::beast::http::status::no_content;
                        case Code::MovePerm :
                            return boost::beast::http::status::moved_permanently;
                        case Code::MoveTemp :
                            return boost::beast::http::status::found;
                        case Code::NotModified :
                            return boost::beast::http::status::not_modified;
                        case Code::BadRequest :
                            return boost::beast::http::status::bad_request;
                        case Code::NotFound :
                            return boost::beast::http::status::not_found;
                        case Code::Unauthorized :
                            return boost::beast::http::status::unauthorized;
                        case Code::BadMethod :
                            return boost::beast::http::status::method_not_allowed;
                        case Code::Internal :
                            return boost::beast::http::status::internal_server_error;
                        case Code::NotImplemented :
                            return boost::beast::http::status::not_implemented;
                        case Code::Unavaliable :
                            return boost::beast::http::status::service_unavailable;
                        default :
                            break;
                        }

                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Utility::ConvertCode] Unknowd HTTP code."};
                    }

                    Code ConvertCode(boost::beast::http::status code)
                    {
                        switch (code)
                        {
                        case boost::beast::http::status::ok :
                            return Code::Ok;
                        case boost::beast::http::status::no_content :
                            return Code::NoContent;
                        case boost::beast::http::status::moved_permanently :
                            return Code::MovePerm;
                        case boost::beast::http::status::found :
                            return Code::MoveTemp;
                        case boost::beast::http::status::not_modified :
                            return Code::NotModified;
                        case boost::beast::http::status::bad_request :
                            return Code::BadRequest;
                        case boost::beast::http::status::not_found :
                            return Code::NotFound;
                        case boost::beast::http::status::unauthorized :
                            return Code::Unauthorized;
                        case boost::beast::http::status::method_not_allowed :
                            return Code::BadMethod;
                        case boost::beast::http::status::internal_server_error :
                            return Code::Internal;
                        case boost::beast::http::status::not_implemented :
                            return Code::NotImplemented;
                        case boost::beast::http::status::service_unavailable :
                            return Code::Unavaliable;
                        default :
                            break;
                        }

                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Utility::ConvertCode] Unknowd HTTP code " +
                                std::to_string(static_cast<unsigned>(code))};
                    }

                    std::string CreateTimestamp()
                    {
                        std::stringstream stream;
                        stream.imbue(std::locale(std::locale::classic(),
                                new boost::posix_time::time_input_facet("%a, %d %b %Y %H:%M:%S GMT")));
                        auto const now = boost::posix_time::second_clock::universal_time();
                        stream << now;
                        return stream.str();
                    }

                    std::string DecodeUrl(std::string const &url)
                    {
                        std::string out;
                        out.reserve(url.size());
                        for (std::size_t i = 0; i < url.size() ; ++i)
                        {
                            if (url[i] == '%')
                            {
                                if (i + 3 <= url.size())
                                {
                                    int value = 0;
                                    std::istringstream is(url.substr(i + 1, 2));
                                    if (is >> std::hex >> value)
                                    {
                                        out += static_cast<char>(value);
                                        i += 2;
                                    }
                                    else
                                    {
                                        throw std::runtime_error{"[Mif::Net::Http::Detail::Utility::DecodeUrl] "
                                                "Failed to decode url. Error: failed to recognize the char."};
                                    }
                                }
                                else
                                {
                                    throw std::runtime_error{"[Mif::Net::Http::Detail::Utility::DecodeUrl] "
                                            "Failed to decode url. Error: bad encoding."};
                                }
                            }
                            else if (url[i] == '+')
                                out += ' ';
                            else
                                out += url[i];
                        }

                        return out;
                    }

                    std::string EncodeUrl(std::string const &url)
                    {
                        std::ostringstream stream;
                        stream.fill('0');
                        stream << std::hex;

                        for (auto i = std::begin(url) ; i != std::end(url) ; ++i)
                        {
                            auto c = *i;

                            if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
                            {
                                stream << c;
                                continue;
                            }

                            stream << std::uppercase;
                            stream << '%' << std::setw(2)
                                   << static_cast<int>(static_cast<unsigned char>(c));
                            stream << std::nouppercase;
                        }

                        return stream.str();
                    }

                    Target::Target (std::string const &url)
                    {
                        std::regex const regexpr{
                                "^(^/+.*?(?=[?#]|$))"    // Path
                                "[?#/]*" // Splitter
                                "(.*)?"    // Query
                                "$"
                            };

                        std::smatch match;

                        if (!std::regex_match(std::begin(url), std::end(url), match, regexpr))
                            throw std::runtime_error{"Failed to parse url \"" + url + "\"."};

                        m_path.assign(match[1].first, match[1].second);
                        m_query.assign(match[2].first, match[2].second);
                    }

                    std::string const& Target::GetPath() const noexcept
                    {
                        return m_path;
                    }

                    std::string const& Target::GetQuery() const noexcept
                    {
                        return m_query;
                    }

                }   // namespace Utility

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
