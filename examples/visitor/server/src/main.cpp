//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/application.h>
#include <mif/common/log.h>
#include <mif/net/tcp_server.h>

// COMMON
#include "common/client.h"
#include "common/ps/iface.h"

// THIS
#include "common/id/service.h"

class Applicatin
    : public Mif::Application::Application
{
public:
    Applicatin(int argc, char const **argv)
        : Mif::Application::Application{argc, argv}
    {
        boost::program_options::options_description options{"Server options"};
        options.add_options()
                ("host", boost::program_options::value<std::string>()->default_value("0.0.0.0"), "Server host")
                ("port", boost::program_options::value<std::string>()->default_value("55555"), "Server port")
                ("workers", boost::program_options::value<std::uint16_t>()->default_value(8), "Workers thread count");

        AddCustomOptions(options);
    }

private:
    std::unique_ptr<Mif::Net::TCPServer> m_server;

    // Mif.Application.Application
    virtual void OnStart() override final
    {
        auto const &options = GetOptions();

        auto const host = options["host"].as<std::string>();
        auto const port = options["port"].as<std::string>();
        auto const workers = options["workers"].as<std::uint16_t>();

        MIF_LOG(Info) << "Starting server on " << host << ":" << port;

        auto factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::Factory>();
        factory->AddClass<::Service::Id::Service>();

        std::chrono::microseconds const timeout{10 * 1000 * 1000};

        auto clientFactory = Service::Ipc::MakeClientFactory(workers, timeout, factory);

        m_server.reset(new Mif::Net::TCPServer{host, port, clientFactory});

        MIF_LOG(Info) << "Server is successfully started.";
    }

    virtual void OnStop() override final
    {
        MIF_LOG(Info) << "Stopping server ...";

        m_server.reset();

        MIF_LOG(Info) << "Server is successfully stopped.";
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Applicatin>(argc, argv);
}
