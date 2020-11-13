//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2018
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_PARAMS_H__
#define __MIF_NET_HTTP_DETAIL_PARAMS_H__

// STD
#include <map>
#include <string>

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {
                namespace Tag
                {

                    struct Params;
                    struct Headers;

                }   // namespace Tag

                template <typename /*tag*/>
                class Params final
                {
                public:
                    using Type = std::map<std::string, std::string>;

                    Params(Type const &params)
                        : m_params{params}
                    {
                    }

                    Type const& Get() const
                    {
                        return m_params;
                    }

                private:
                    Type const &m_params;
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_PARAMS_H__
