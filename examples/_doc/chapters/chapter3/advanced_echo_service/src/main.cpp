// STD
#include <algorithm>
#include <iterator>
#include <set>
#include <string>

// BOOST
#include <boost/algorithm/string.hpp>

// MIF
#include <mif/application/http_server.h>
#include <mif/net/http/make_web_service.h>
#include <mif/net/http/web_service.h>

class EchoService
    : public Mif::Net::Http::WebService
{
public:
    EchoService(std::string const &locationPrefix)
    {
        AddHandler(locationPrefix + "/text", this, &EchoService::EchoText);
        AddHandler(locationPrefix + "/headers", this, &EchoService::EchoHeaders);
        AddHandler(locationPrefix + "/params", this, &EchoService::EchoParams);
        AddHandler(locationPrefix + "/prm", this, &EchoService::EchoPrm);
        AddHandler(locationPrefix + "/stat", this, &EchoService::Stat);
    }

private:
    std::string EchoText(Content<std::string> const &content)
    {
        return "Echo from server: \"" + content.Get() + "\"\n";
    }

    std::string EchoHeaders(Headers const &headers)
    {
        return "Echo from server:\n" + ToString(headers.Get()) + "\n";
    }

    std::string EchoParams(Params const &params)
    {
        return "Echo from server:\n" + ToString(params.Get()) + "\n";
    }

    std::string EchoPrm(Prm<std::string, Name("prm1")> const &prm1,
                        Prm<int, Name("prm2")> const &prm2,
                        Prm<std::set<std::string>, Name("prm3")> const &prm3)
    {
        if (!prm1)
            throw std::invalid_argument{"No prm1."};
        if (!prm2)
            throw std::invalid_argument{"No prm2."};
        if (!prm3)
            throw std::invalid_argument{"No prm3."};
        return "Echo from server:\n"
                "\tPrm1: " + prm1.Get() + "\n"
                "\tPrm2: " + std::to_string(prm2.Get()) + "\n"
                "\tPrm3: (" + boost::algorithm::join(prm3.Get(), ",") + ")\n";
    }

    std::string Stat()
    {
        std::set<std::string> items;
        auto const &stat = GetStatistics();
        std::transform(std::begin(stat.resources), std::end(stat.resources),
                std::inserter(items, std::begin(items)),
                [] (typename std::decay<decltype(stat.resources)>::type::value_type const &i)
                {
                    std::string item = "Resource: " + i.first + "\n";
                    item += "\tTotal: " + std::to_string(i.second.total) + "\n";
                    item += "\tBad: " + std::to_string(i.second.bad) + "\n";
                    return item;
                }
            );
        return boost::algorithm::join(items, "\n");
    }

    std::string ToString(std::map<std::string, std::string> const &map) const
    {
        std::set<std::string> items;
        std::transform(std::begin(map), std::end(map),
                std::inserter(items, std::begin(items)),
                [] (typename std::decay<decltype(map)>::type::value_type const &i)
                {
                    return i.first + ": " + i.second;
                }
            );
        return boost::algorithm::join(items, "\n");
    }
};

class Application
    : public Mif::Application::HttpServer
{
public:
    using HttpServer::HttpServer;

private:
    virtual void Init(Mif::Net::Http::ServerHandlers &handlers) override final
    {
        auto const echoServiceLocation = "/echo";
        auto echoService = Mif::Net::Http::MakeWebService<EchoService>(echoServiceLocation);
        handlers.emplace(echoServiceLocation, std::move(echoService));
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
