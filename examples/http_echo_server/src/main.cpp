//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/http_server.h>
#include <mif/common/log.h>
#include <mif/net/http/constants.h>

class Application
    : public Mif::Application::HttpServer
{
public:
    using HttpServer::HttpServer;

private:
    // Mif.Application.HttpServer
    virtual void Init(Mif::Net::Http::ServerHandlers &handlers) override final
    {
        handlers["/"] = [] (Mif::Net::Http::IInputPack const &request,
                Mif::Net::Http::IOutputPack &response)
        {
            auto const data = request.GetData();

            MIF_LOG(Info) << "Received bytes: " << data.size();

            response.SetCode(Mif::Net::Http::Code::Ok);

            response.SetData(std::move(data));
        };
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
