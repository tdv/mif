// STD
#include <atomic>
#include <string>

// MIF
#include <mif/application/http_server.h>
#include <mif/net/http/web_service.h>
#include <mif/net/http/make_web_service.h>

class EchoService
    : public Mif::Net::Http::WebService
{
public:
    EchoService(std::string const &locationPrefix)
    {
        AddHandler(locationPrefix + "", this, &EchoService::Echo);
        AddHandler(locationPrefix + "/stat", this, &EchoService::Stat);
    }

private:
    std::atomic_size_t m_counter{0};

    std::string Echo(Content<std::string> const &content)
    {
        ++m_counter;
        return "Echo from server: \"" + content.Get() + "\"\n";
    }

    std::string Stat()
    {
        return "Count of echo-request: " + std::to_string(m_counter) + "\n";
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
