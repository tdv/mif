//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

// STD
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>

// MIF
#include "mif/net/http/serializer/json.h"
#include "mif/net/http/serializer/plain_text.h"
#include "mif/net/http/web_service.h"
#include <mif/service/creator.h>

// COMMON
#include "common/id/service.h"
#include "common/interface/iadmin.h"

namespace Service
{
    namespace Detail
    {
        namespace
        {

            class WebService
                : public Mif::Service::Inherit
                    <
                        IAdmin,
                        Mif::Net::Http::WebService
                    >
            {
            public:

                WebService(std::string const &pathPrefix)
                {
                    AddHandler(pathPrefix + "/stat", this, &WebService::Stat);
                    AddHandler(pathPrefix + "/main-page", this, &WebService::MainPage);
                }

            private:
                using LockType = std::mutex;
                using LockGuard = std::lock_guard<LockType>;

                mutable LockType m_lock;

                std::string m_title = "Test page";
                std::string m_body = "Test page data";

                // IAdmin
                virtual void SetTitle(std::string const &title) override final
                {
                    LockGuard lock{m_lock};
                    m_title = title;
                }

                virtual void SetBody(std::string const &body) override final
                {
                    LockGuard lock{m_lock};
                    m_body = body;
                }

                virtual std::string GetPage() const override final
                {
                    LockGuard lock{m_lock};
                    return "Titlt:\n" + m_title + "\n\nBody:\n" + m_body + "\n";
                }

                // Web hadlers
                Result<Mif::Net::Http::Serializer::Json> Stat()
                {
                    auto const &statisticts = GetStatistics();

                    std::map<std::string, std::int64_t> resp;

                    resp["Total requests "] = statisticts.general.total;
                    resp["Bad requests "] = statisticts.general.bad;

                    for (auto const &s : statisticts.resources)
                    {
                        resp["Total requests for '" + s.first + "' "] = s.second.total;
                        resp["Bad requests for '" + s.first + "' "] = s.second.bad;
                    }

                    return resp;
                }

                Result<Mif::Net::Http::Serializer::PlainText>
                MainPage(Prm<std::string, Name("format")> const &format)
                {
                    if (!format)
                        throw std::invalid_argument{"No 'format'"};

                    std::string title;
                    std::string body;

                    {
                        LockGuard lock{m_lock};
                        title = m_title;
                        body = m_body;
                    }

                    if (format.Get() == "json")
                    {
                        auto const data = std::make_pair(title, body);
                        auto const json = Mif::Serialization::Json::Serialize(data);
                        return {std::string{std::begin(json), std::end(json)}, "application/json;charset=UTF-8"};
                    }
                    else if (format.Get() == "html")
                    {
                        std::stringstream html;
                        html << "<!DOCTYPE html>"
                             << "<html>"
                             << "<head>"
                             << "<title>" << title << "</title>"
                             << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
                             << "<meta name=\"resourse-type\" content=\"document\">"
                             << "</head>"
                             << "<body>"
                             << body
                             << "</body>\n";


                        return {html.str(), "text/html"};
                    }
                    else if (format.Get() == "text")
                    {
                        std::stringstream text;
                        text << "Title:\n" << title << "\n\n"
                             << "Body:\n" << body << "\n\n";
                        return {text.str(), "text/plain; charset=utf-8"};
                    }

                    throw std::invalid_argument{"Format '" + format.Get() + "' not supported."};
                }
            };

        }   // namespace
    }   // namespace Detail
}   // namespace Service

MIF_SERVICE_CREATOR
(
    ::Service::Id::Service,
    ::Service::Detail::WebService,
    std::string
)
