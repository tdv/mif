// STD
#include <string>
#include <vector>

// MIF
#include <mif/application/http_server.h>
#include <mif/common/log.h>
#include <mif/net/http/make_web_service.h>
#include <mif/net/http/converter/content/json.h>
#include <mif/net/http/converter/content/xml.h>
#include <mif/net/http/serializer/json.h>
#include <mif/net/http/serializer/xml.h>
#include <mif/net/http/web_service.h>
#include <mif/reflection/reflect_type.h>
#include <mif/reflection/reflection.h>

namespace Data
{

    enum class Type
    {
        Unknown,
        Type1,
        Type2
    };

    struct Item
    {
        Type type = Type::Unknown;
        std::string description;
    };

    struct Collection
    {
        std::string name;
        std::vector<Item> items;
    };

}   // namespace Data

namespace Data
{
    namespace Meta
    {

        MIF_REFLECT_BEGIN(Type)
            MIF_REFLECT_FIELD(Unknown)
            MIF_REFLECT_FIELD(Type1)
            MIF_REFLECT_FIELD(Type2)
        MIF_REFLECT_END()

        MIF_REFLECT_BEGIN(Item)
            MIF_REFLECT_FIELD(type)
            MIF_REFLECT_FIELD(description)
        MIF_REFLECT_END()

        MIF_REFLECT_BEGIN(Collection)
            MIF_REFLECT_FIELD(name)
            MIF_REFLECT_FIELD(items)
        MIF_REFLECT_END()

    }   // namespace Meta
}   // namespace Data

MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Type)
MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Item)
MIF_REGISTER_REFLECTED_TYPE(Data::Meta::Collection)

class ConvertService
    : public Mif::Net::Http::WebService
{
public:
    ConvertService(std::string const &locationPrefix)
    {
        AddHandler(locationPrefix + "/json/to/xml", this, &ConvertService::JsonToXml);
        AddHandler(locationPrefix + "/xml/to/json", this, &ConvertService::XmlToJson);
    }

private:
    template <typename T>
    using JsonContent = Content<T, Mif::Net::Http::Converter::Content::Json>;
    template <typename T>
    using XmlContent = Content<T, Mif::Net::Http::Converter::Content::Xml>;

    using JsonResponse = Result<Mif::Net::Http::Serializer::Json>;
    using XmlResponse = Result<Mif::Net::Http::Serializer::Xml>;

    XmlResponse JsonToXml(JsonContent<Data::Collection> const &content)
    {
        auto const &data = content.Get();
        MIF_LOG(Trace) << "Converting \"" << data.name << "\" from json to xml.";
        return data;
    }

     JsonResponse XmlToJson(XmlContent<Data::Collection> const &content)
     {
        auto const &data = content.Get();
        MIF_LOG(Trace) << "Converting \"" << data.name << "\" from xml to json.";
        return data;
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
        auto const convertServiceLocation = "/convert";
        auto convertService = Mif::Net::Http::MakeWebService<ConvertService>(convertServiceLocation);
        handlers.emplace(convertServiceLocation, std::move(convertService));
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
