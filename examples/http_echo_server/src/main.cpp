//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2017 tdv
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
            auto data = request.GetData();

            MIF_LOG(Info) << "Process request \"" << request.GetPath() << request.GetQuery() << "\"\t Data: "
                    << (data.empty() ? std::string{"null"} : std::string{std::begin(data), std::end(data)});

            response.SetCode(Mif::Net::Http::Code::Ok);
            response.SetHeader(Mif::Net::Http::Constants::Header::Connection::Value,
                               Mif::Net::Http::Constants::Value::Connection::Close::Value);

            response.SetData(std::move(data));
        };
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
