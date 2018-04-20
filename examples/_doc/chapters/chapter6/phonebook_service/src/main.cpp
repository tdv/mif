// MIF
#include <mif/application/http_server.h>
#include <mif/net/http/make_web_service.h>
#include <mif/service/factory.h>
#include <mif/service/make.h>
#include <mif/service/root_locator.h>

// THIS
#include "id/service.h"

namespace Phonebook
{

    class Application
        : public Mif::Application::HttpServer
    {
    public:
        using HttpServer::HttpServer;

    private:
        virtual void Init(Mif::Net::Http::ServerHandlers &handlers) override final
        {
            auto config = GetConfig();
            if (!config)
                throw std::runtime_error{"No input config."};

            auto storageConfig = config->GetConfig("storage");
            if (!storageConfig)
                throw std::runtime_error{"No \"storage\" node in the \"data\" section in config."};

            auto factory = Mif::Service::Make<Mif::Service::Factory, Mif::Service::Factory>();

            factory->AddClass<Id::PgStorage>(storageConfig->GetConfig("pg"));
            factory->AddClass<Id::FileStorage>(storageConfig->GetConfig("file"));

            auto const activeStorageId = Mif::Common::Crc32str(storageConfig->GetValue("active.id"));
            auto storage = factory->Create(activeStorageId);

            Mif::Service::RootLocator::Get()->Put(Id::Storage, storage);

            std::string const phonebookLocation = "/phonebook";
            handlers.emplace(phonebookLocation, Mif::Net::Http::MakeWebService<Id::Phonebook>(phonebookLocation));
        }
    };

}   // namespace Phonebook

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Phonebook::Application>(argc, argv);
}
