//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_INPUT_PACK_H__
#define __MIF_NET_HTTP_DETAIL_INPUT_PACK_H__

// STD
#include <deque>
#include <memory>
#include <iterator>
#include <set>
#include <string>

// BOOST
#include <boost/algorithm/string.hpp>
#include <boost/beast/http.hpp>

// MIF
#include "mif/net/http/iinput_pack.h"

// THIS
#include "utility.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                template <typename>
                class InputPack;

                template <typename TBody, typename TAllocator>
                class InputPack<boost::beast::http::request<TBody, boost::beast::http::basic_fields<TAllocator>>> final
                    : public IInputPack
                {
                public:
                    using Request = boost::beast::http::request<TBody, boost::beast::http::basic_fields<TAllocator>>;

                    InputPack(Request const &request)
                        : m_request{request}
                        , m_target{std::string{m_request.target()}}
                    {
                    }

                private:
                    Request const &m_request;
                    Utility::Target m_target;

                    // IInputPack
                    virtual Method::Type GetType() const override final
                    {
                        return Utility::ConvertMethodType(m_request.method());
                    }

                    virtual std::string GetPath() const override final
                    {
                        return m_target.GetPath();
                    }

                    virtual std::string GetQuery() const override final
                    {
                        return Utility::DecodeUrl(m_target.GetQuery());
                    }

                    virtual Params GetParams() const override final
                    {
                        Params params;

                        auto query = GetQuery();

                        if (query.empty())
                            return params;

                        if (query.at(0) == '?')
                            query.erase(0, 1);

                        std::set<std::string> pairs;
                        boost::algorithm::split(pairs, query, boost::is_any_of("&"));
                        if (pairs.empty())
                            return params;

                        std::transform(std::begin(pairs), std::end(pairs),
                                std::inserter(params, std::begin(params)),
                                [] (std::string const &pair) -> std::pair<std::string, std::string>
                                {
                                    std::deque<std::string> prm;
                                    boost::algorithm::split(prm, pair, boost::is_any_of("="));
                                    if (prm.size() == 2)
                                        return std::make_pair(prm.front(), prm.back());
                                    if (prm.size() == 1)
                                        return std::make_pair(prm.front(), std::string{});
                                    return {};
                                }
                            );

                        return params;
                    }

                    virtual Headers GetHeaders() const override final
                    {
                        Headers headers;

                        for (auto const &i : m_request.base())
                            headers.emplace(std::string{i.name_string()}, std::string{i.value()});

                        return headers;
                    }

                    virtual Common::Buffer const& GetData() const override final
                    {
                        return m_request.body();
                    }
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_INPUT_PACK_H__
