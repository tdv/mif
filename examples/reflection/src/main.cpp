//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     08.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>
#include <map>
#include <string>
#include <type_traits>

// MIF
#include <mif/reflection/reflect_type.h>
#include <mif/reflection/reflection.h>
#include <mif/serialization/traits.h>

struct Base1
{
    int field1 = 0;
    bool field2 = false;
};

struct Base2
{
    std::string field3;
};

struct Nested
{
    int field = 0;
};

struct Data : Base1, Base2
{
    int field4 = 0;
    std::string field5;
    std::map<std::string, Nested> field6;
};

MIF_REFLECT_BEGIN(Base1)
    MIF_REFLECT_FIELD(field1)
    MIF_REFLECT_FIELD(field2)
MIF_REFLECT_END()

MIF_REFLECT_BEGIN(Base2)
    MIF_REFLECT_FIELD(field3)
MIF_REFLECT_END()

MIF_REFLECT_BEGIN(Nested)
    MIF_REFLECT_FIELD(field)
MIF_REFLECT_END()

MIF_REFLECT_BEGIN(Data, Base1, Base2)
    MIF_REFLECT_FIELD(field4)
    MIF_REFLECT_FIELD(field5)
    MIF_REFLECT_FIELD(field6)
MIF_REFLECT_END()

MIF_REGISTER_REFLECTED_TYPE(Base1)
MIF_REGISTER_REFLECTED_TYPE(Base2)
MIF_REGISTER_REFLECTED_TYPE(Nested)
MIF_REGISTER_REFLECTED_TYPE(Data)

class Printer final
{
public:

    template <typename T>
    static
    typename std::enable_if<Mif::Reflection::IsReflectable<T>(), void>::type
    Print(T const &data)
    {
        using Meta = Mif::Reflection::Reflect<T>;
        using Base = typename Meta::Base;
        PrintBase<0, std::tuple_size<Base>::value, Base>(data);
        std::cout << "Struct name: " << Meta::Name::GetString() << std::endl;
        Print<0, Meta::Fields::Count>(data);
    }

    template <typename T>
    static
    typename std::enable_if
        <
            !Mif::Reflection::IsReflectable<T>() && !Mif::Serialization::Traits::IsIterable<T>(),
            void
        >::type
    Print(T const &data)
    {
        std::cout << data << std::boolalpha << std::endl;
    }

    template <typename T>
    static
    typename std::enable_if
        <
            !Mif::Reflection::IsReflectable<T>() && Mif::Serialization::Traits::IsIterable<T>(),
            void
        >::type
    Print(T const &data)
    {
        for (auto const &i : data)
            Print(i);
    }

private:
    template <std::size_t I, std::size_t N, typename T>
    static
    typename std::enable_if<I != N, void>::type
    Print(T const &data)
    {
        using Meta = Mif::Reflection::Reflect<T>;
        using Field = typename Meta::Fields::template Field<I>;
        std::cout << Field::Name::GetString() << " = ";
        Print(data.*Field::Access());
        Print<I + 1, N>(data);
    }

    template <std::size_t I, std::size_t N, typename T>
    static
    typename std::enable_if<I == N, void>::type
    Print(T const &)
    {
    }

    template <typename K, typename V>
    static void Print(std::pair<K, V> const &p)
    {
        Print(p.first);
        Print(p.second);
    }

    template <std::size_t I, std::size_t N, typename B, typename T>
    static
    typename std::enable_if<I != N, void>::type
    PrintBase(T const &data)
    {
        using Type = typename std::tuple_element<I, B>::type;
        Print(static_cast<Type const &>(data));
        PrintBase<I + 1, N, B>(data);
    }

    template <std::size_t I, std::size_t N, typename B, typename T>
    static
    typename std::enable_if<I == N, void>::type
    PrintBase(T const &)
    {
    }
};

int main()
{
    Data data;

    data.field1 = 1;
    data.field2 = true;
    data.field3 = "Text";
    data.field4 = 100;
    data.field5 = "String";
    data.field6["key1"].field = 100;
    data.field6["key2"].field = 200;

    Printer::Print(data);

    return 0;
}
