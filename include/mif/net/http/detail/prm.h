//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2018
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_PRM_H__
#define __MIF_NET_HTTP_DETAIL_PRM_H__

// STD
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                template
                <
                    typename T,
                    std::uint32_t ID,
                    typename TConverter
                >
                class Prm final
                {
                public:
                    using PrmType = Prm<T, ID, TConverter>;
                    using Type = T;
                    static constexpr std::uint32_t Id = ID;
                    using Converter = TConverter;

                    Prm() = default;
                    Prm(Prm &&) = default;
                    Prm(Prm const &) = delete;

                    Prm(std::string const &name, std::string const &value)
                        : m_name{name}
                        , m_value{new T{Converter::template Convert<T>(value)}}
                    {
                    }

                    explicit operator bool() const noexcept
                    {
                        return !!m_value;
                    }

                    Type const& Get() const
                    {
                        if (!*this)
                            throw std::invalid_argument{"[Mif::Net::Http::WebService::Prm] \"" + m_name + "\" has no value."};
                        return *m_value;
                    }

                private:
                    std::string m_name;
                    std::unique_ptr<T> const m_value;
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_PRM_H__
