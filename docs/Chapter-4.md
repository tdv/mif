# Web service. Content processing
The chapter will be devoted to the automatic extraction of user-defined data structures from the request body. In this chapter will be created a Web service for convertation user-defined data structures between the json and xml formats.  

The following code defines user data
```cpp
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
```
This is a very simple data structure, but it allows you to demonstrate the work with enums, structs, nested structs and containers.  

The current C++ standard has no  support for reflection. I waited for it on C++ 11/14/17. Well, I made an implementation for this. MIF is metainfo framework. Everything in it is done around the meta-information about user data structures. To use reflection in MIF, you must include one header file and add meta information to your types by macros.  

The following code demonstrate the addition of meta information to types
```cpp
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
```
To use reflection you must:
- include the header file
```cpp
#include <mif/reflection/reflect_type.h>
```
- add meta information with macros MIF_REFLECT_BEGIN, MIF_REFLECT_FIELD, MIF_REFLECT_END
- register the added meta information with the macro MIF_REGISTER_REFLECTED_TYPE  

After that you can:
- serialize and deserialize your own types
- use as parameters in your own web services
- use as parameters in your own services (not in web services)  

Take a look at the following code
```cpp
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
```
This is a very simple web service for converting input user-defined data in json format to xml and back.  
Look again at this code:
```cpp
template <typename T>
using JsonContent = Content<T, Mif::Net::Http::Converter::Content::Json>;
template <typename T>
using XmlContent = Content<T, Mif::Net::Http::Converter::Content::Xml>;
```
In the previous chapter, the Content class was considered. It was used to simply extract data in the form of a raw string. In the current example, the Content class is used with serializers to extract data from the json and xml formats.  
Look again at this code:
```cpp
using JsonResponse = Result<Mif::Net::Http::Serializer::Json>;
using XmlResponse = Result<Mif::Net::Http::Serializer::Xml>;
```
In this code, you will see a new Result class, which you can use to create a response in the format you need. This class is defined in the WebService class.  

Starting the service
```bash
./content_convert_service --daemon
```
For testing you should to create data.json file with following content
```json
{
	"name": "Test",
	"items": [{
		"type": "Type1",
		"description": "Description for Type1"
	}, {
		"type": "Type2",
		"description": "Description for Type2"
	}]
}
```
Test of the conversion from json to xml of the user-defined data structure:
```bash
curl -is "http://localhost:55555/convert/json/to/xml" -d @data.json 
HTTP/1.1 200 OK
Content-Type: text/xml; charset=UTF-8
Date: Wed, 18 Apr 2018 09:57:00 GMT
Content-Length: 326

<?xml version="1.0" encoding="UTF-8"?>
<Collection>
	<name><![CDATA[Test]]></name>
	<items>
		<item>
			<type>Type1</type>
			<description><![CDATA[Description for Type1]]></description>
		</item>
		<item>
			<type>Type2</type>
			<description><![CDATA[Description for Type2]]></description>
		</item>
	</items>
</Collection>
```
Save the response as a data.xml file:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<Collection>
	<name><![CDATA[Test]]></name>
	<items>
		<item>
			<type>Type1</type>
			<description><![CDATA[Description for Type1]]></description>
		</item>
		<item>
			<type>Type2</type>
			<description><![CDATA[Description for Type2]]></description>
		</item>
	</items>
</Collection>
```
And try again to call the service to convert from xml to json:
```bash
curl -is "http://localhost:55555/convert/xml/to/json" -d @data.xml
HTTP/1.1 200 OK
Content-Type: application/json; charset=UTF-8
Date: Wed, 18 Apr 2018 10:03:07 GMT
Content-Length: 184

{
	"items" : 
	[
		{
			"description" : "Description for Type1",
			"type" : "Type1"
		},
		{
			"description" : "Description for Type2",
			"type" : "Type2"
		}
	],
	"name" : "Test"
}
```
Stopping the service:
```bash
sudo killall -w content_convert_service
```

Congratulations. Now you have all the necessary knowledge to process the data in your own web service, created on MIF.  

**All the code you will find in the example** 
[content_convert_service](https://github.com/tdv/mif/tree/master/examples/_doc/chapters/chapter4/content_convert_service)