//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_UTILITY_H__
#define __MIF_NET_HTTP_DETAIL_UTILITY_H__

// BOOST
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>

// MIF
#include "mif/net/http/codes.h"
#include "mif/net/http/methods.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {
                namespace Utility
                {

                    Method::Type ConvertMethodType(boost::beast::http::verb verb);

                    char const* GetReasonString(Code code);
                    boost::beast::http::status ConvertCode(Code code);
                    Code ConvertCode(boost::beast::http::status code);

                    class Target final
                    {
                    public:
                        Target(std::string const &url);

                        std::string const& GetPath() const noexcept;
                        std::string const& GetQuery() const noexcept;

                    private:
                        std::string m_path;
                        std::string m_query;
                   };

                }   // namespace Utility

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif // !__MIF_NET_HTTP_DETAIL_UTILITY_H__
