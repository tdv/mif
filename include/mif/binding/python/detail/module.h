//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     04.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_BINDING_PYTHON_DETAIL_MODULE_H__
#define __MIF_BINDING_PYTHON_DETAIL_MODULE_H__

// STD
#include <list>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

// BOOST
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/regex.h>

// MIF
#include "mif/reflection/reflection.h"

#include <iostream>
namespace Mif
{
    namespace Binding
    {
        namespace Python
        {
            namespace Detail
            {

                template <std::size_t>
                struct Dummy
                {
                };

                struct NameNode final
                {
                    using Items = std::map<std::string, NameNode>;

                    Items items;
                };

                class ScopeTreeGenerator final
                {
                public:
                    using ScopePtr = std::unique_ptr<boost::python::scope>;

                    void Generate(NameNode::Items const &items)
                    {
                        std::list<ScopePtr> scopes;

                        for (auto const &item : items)
                        {
                            ++m_id;
                            scopes.emplace_back(GenerateScope(m_id, item.first.c_str()));
                            Generate(item.second.items);
                        }
                    }

                private:
                    std::size_t m_id = 0;

                    template <std::size_t I>
                    typename std::enable_if<I != Common::Detail::FakeHierarchyLength::value, ScopePtr>::type
                    GenerateScope(std::size_t id, char const *name)
                    {
                        if (I != id)
                            return GenerateScope<I + 1>(id, name);
                        return ScopePtr{new boost::python::scope{boost::python::class_<Dummy<I>, boost::noncopyable>{
                                name, boost::python::no_init}}};
                    }

                    template <std::size_t I>
                    typename std::enable_if<I == Common::Detail::FakeHierarchyLength::value, ScopePtr>::type
                    GenerateScope(std::size_t, char const *name)
                    {
                        throw std::logic_error{"[Mif::Binding::Python::Detail::ScopeTreeGenerator::GenerateScope] "
                                "Failed to create scope \"" + std::string{name} + "\". Too many scopes."};
                    }

                    ScopePtr GenerateScope(std::size_t id, char const *name)
                    {
                        return GenerateScope<0>(id, name);
                    }
                };

                inline std::list<std::string> ParseFullName(std::string fullName)
                {
                    std::list<std::string> names;
                    boost::algorithm::trim_left_if(fullName, [] (char ch) { return ch == ':'; } );
                    boost::algorithm::split_regex(names, fullName, boost::regex{"::"});
                    std::for_each(std::begin(names), std::end(names),
                            [] (std::string &s) { boost::algorithm::trim(s); } );
                    auto end = std::remove_if(std::begin(names), std::end(names),
                            [] (std::string const &s) { return s.empty(); } );
                    names.erase(end, std::end(names));
                    return names;
                }

                struct NSTreeVisitor final
                {
                    using Result = bool;

                    template <typename T>
                    static bool Visit(NameNode::Items *items)
                    {
                        using Type = Reflection::Reflect<T>;

                        auto names = ParseFullName(Type::FullName::GetString());
                        if (names.size() < 2)
                            return false;

                        names.pop_back();
                        for (auto const &name : names)
                        {
                            auto iter = items->find(name);
                            if (iter == std::end(*items))
                                items = &(*items)[name].items;
                            else
                                items = &iter->second.items;
                        }

                        return false;
                    }
                };

                template <typename T, std::size_t I, std::size_t Count>
                struct DefField final
                {
                    template <typename TClass>
                    static void Def(TClass &cls)
                    {
                        using Field = typename T::template Field<I>;
                        cls.def_readwrite(Field::Name::GetString().c_str(), Field::Access());
                        DefField<T, I + 1, Count>::Def(cls);
                    }
                };

                template <typename T, std::size_t I>
                struct DefField <T, I, I> final
                {
                    template <typename TClass>
                    static void Def(TClass &)
                    {
                    }
                };

                struct StructVisitor final
                {
                    using Result = bool;

                    template <typename T>
                    static bool Visit()
                    {
                        using Type = Reflection::Reflect<T>;

                        auto const names = ParseFullName(Type::FullName::GetString());
                        if (names.empty())
                            return false;

                        std::shared_ptr<boost::python::class_<T>> cls;
                        if (names.size() == 1)
                        {
                            cls = std::make_shared<boost::python::class_<T>>(names.front().c_str());
                        }
                        else
                        {
                            boost::python::object object = boost::python::scope{};
                            for (auto i = std::begin(names) ; i != std::end(names) ; )
                            {
                                auto cur = i;
                                if (++i == std::end(names))
                                {
                                    boost::python::scope scope{object};
                                    cls = std::make_shared<boost::python::class_<T>>(cur->c_str());
                                }
                                else
                                    object = object.attr(cur->c_str());
                            }
                        }

                        using Fields = typename Type::Fields;
                        DefField<Fields, 0, Fields::Count>::Def(*cls);

                        return false;
                    }
                };

                inline void ImportStructs()
                {
                    NameNode::Items nsTree;
                    Reflection::Detail::Registry::Visitor::Accept<NSTreeVisitor>(&nsTree);
                    ScopeTreeGenerator{}.Generate(nsTree);
                    Reflection::Detail::Registry::Visitor::Accept<StructVisitor>();
                }

            }   // namespace Detail
        }   // namespace Python
    }   // namespace Binding
}   // namespace Mif

#endif  // !__MIF_BINDING_PYTHON_DETAIL_MODULE_H__
