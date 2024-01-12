//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2017
//  Copyright (C) 2016-2024 tdv
//-------------------------------------------------------------------

#ifndef __MIF_REMOTE_SERIALIZATION_XML_H__
#define __MIF_REMOTE_SERIALIZATION_XML_H__

// STD
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

// BOOST
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/stream.hpp>

// PUGIXML
#include <pugixml.hpp>

// MIF
#include "mif/common/types.h"
#include "mif/remote/serialization/detail/tag.h"
#include "mif/serialization/xml.h"

namespace Mif
{
    namespace Remote
    {
        namespace Serialization
        {
            namespace Xml
            {

                class Serializer final
                {
                public:
                    template <typename ... TParams>
                    Serializer(bool isReques, std::string const &uuid,
                        std::string const &instanceId, std::string const &interfaceId,
                        std::string const &methodId, TParams && ... params)
                    {
                        {
                            auto decl = m_doc.append_child(pugi::xml_node_type::node_declaration);
                            decl.append_attribute("version") = "1.0";
                            decl.append_attribute("encoding") = "UTF-8";
                        }

                        m_root = m_doc.append_child(Detail::Tag::Pack::Value);

                        m_root.append_child(Detail::Tag::Uuid::Value)
                                .append_child(pugi::xml_node_type::node_pcdata)
                                .set_value(uuid.c_str());

                        m_root.append_child(Detail::Tag::Type::Value)
                                .append_child(pugi::xml_node_type::node_pcdata)
                                .set_value(isReques ? Detail::Tag::Request::Value : Detail::Tag::Response::Value);

                        m_root.append_child(Detail::Tag::Instsnce::Value)
                                .append_child(pugi::xml_node_type::node_pcdata)
                                .set_value(instanceId.c_str());

                        m_root.append_child(Detail::Tag::Interface::Value)
                                .append_child(pugi::xml_node_type::node_pcdata)
                                .set_value(interfaceId.c_str());

                        m_root.append_child(Detail::Tag::Method::Value)
                                .append_child(pugi::xml_node_type::node_pcdata)
                                .set_value(methodId.c_str());

                        PutParamsIfExists(std::forward<TParams>(params) ... );
                    }

                    template <typename ... TParams>
                    void PutParams(TParams && ... params)
                    {
                        m_root.remove_child(Detail::Tag::Param::Value);

                        auto const tuple = std::make_tuple(std::forward<TParams>(params) ... );
                        ::Mif::Serialization::Xml::Detail::Serialize(m_root, tuple, Detail::Tag::Param::Value);
                    }

                    void PutException(std::exception_ptr ex)
                    {
                        m_root.remove_child(Detail::Tag::Exception::Value);

                        try
                        {
                            std::rethrow_exception(ex);
                        }
                        catch (std::exception const &e)
                        {
                            m_root.append_child(Detail::Tag::Exception::Value)
                                    .append_child(pugi::xml_node_type::node_pcdata)
                                    .set_value(e.what());
                        }
                        catch (...)
                        {
                            m_root.append_child(Detail::Tag::Exception::Value)
                                    .append_child(pugi::xml_node_type::node_pcdata)
                                    .set_value("Unknown exception.");
                        }
                    }

                    Common::Buffer GetBuffer()
                    {
                        Common::Buffer buffer;

                        {
                            boost::iostreams::filtering_ostream stream{boost::iostreams::back_inserter(buffer)};
                            m_doc.save(stream);
                            stream.flush();
                        }

                        return buffer;
                    }

                private:
                     pugi::xml_document m_doc;
                     pugi::xml_node m_root;

                    template <typename ... TParams>
                    typename std::enable_if<sizeof ... (TParams), void>::type
                    PutParamsIfExists(TParams && ... params)
                    {
                        PutParams(std::forward<TParams> (params) ... );
                    }

                    template <typename ... TParams>
                    typename std::enable_if< !sizeof ... (TParams), void>::type
                    PutParamsIfExists(TParams && ...)
                    {
                    }
                };

                class Deserializer final
                {
                public:
                    Deserializer(Common::Buffer buffer)
                    {
                        if (buffer.empty())
                            throw std::invalid_argument{"[Mif::Remote::Serialization::Xml::Deserializer] Empty buffer."};

                        {
                            using SourceType = boost::iostreams::basic_array_source<char>;
                            SourceType source{!buffer.empty() ? buffer.data() : nullptr, buffer.size()};
                            boost::iostreams::stream<SourceType> stream{source};
                            auto result = m_doc.load(stream);
                            if (!result)
                            {
                                throw std::invalid_argument{"[Mif::Remote::Serialization::Xml::Deserializer] Bad xml. "
                                        "Error" + std::string{result.description()}};
                            }

                            m_root = m_doc.child(Detail::Tag::Pack::Value);
                        }
                    }

                    std::string const GetUuid() const
                    {
                        return m_root.child(Detail::Tag::Uuid::Value).child_value();
                    }

                    bool IsRequest() const
                    {
                        return GetType() == Detail::Tag::Request::Value;
                    }

                    bool IsResponse() const
                    {
                        return GetType() == Detail::Tag::Response::Value;
                    }

                    std::string const GetType() const
                    {
                        return m_root.child(Detail::Tag::Type::Value).child_value();
                    }

                    std::string const GetInstance() const
                    {
                        return m_root.child(Detail::Tag::Instsnce::Value).child_value();
                    }

                    std::string const GetInterface() const
                    {
                        return m_root.child(Detail::Tag::Interface::Value).child_value();
                    }

                    std::string const GetMethod() const
                    {
                        return m_root.child(Detail::Tag::Method::Value).child_value();
                    }

                    template <typename ... TParams>
                    std::tuple<typename std::decay<TParams>::type ... > GetParams() const
                    {
                        return GetParamsIfExists<TParams ... >();
                    }

                    bool HasException() const
                    {
                        return m_root.child(Detail::Tag::Exception::Value);
                    }

                    std::exception_ptr GetException() const
                    {
                        std::exception_ptr ex{};

                        if (HasException())
                        {
                            try
                            {
                                auto const *message = m_root.child(Detail::Tag::Exception::Value).child_value();
                                throw std::runtime_error{message};
                            }
                            catch (...)
                            {
                                ex = std::current_exception();
                            }
                        }

                        return ex;
                    }

                private:
                    pugi::xml_document m_doc;
                    pugi::xml_node m_root;

                    template <typename ... TParams>
                    typename std::enable_if<sizeof ... (TParams), std::tuple<typename std::decay<TParams>::type ... >>::type
                    GetParamsIfExists() const
                    {
                        using TResult = std::tuple<typename std::decay<TParams>::type ... >;
                        TResult res;
                        ::Mif::Serialization::Xml::Detail::Deserialize(m_root, res, Detail::Tag::Param::Value);
                        return res;
                    }

                    template <typename ... TParams>
                    typename std::enable_if< !sizeof ... (TParams), std::tuple<>>::type
                    GetParamsIfExists() const
                    {
                        return {};
                    }
                };

            }   // namespace Xml
        }   // namespace Serialization
    }   //  namespace Remote
}   // namespace Mif


#endif  // !__MIF_REMOTE_SERIALIZATION_XML_H__
