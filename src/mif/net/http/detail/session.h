//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2020
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_SESSION_H__
#define __MIF_NET_HTTP_DETAIL_SESSION_H__

// STD
#include <chrono>
#include <cstdint>
#include <memory>

// BOOST
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/optional.hpp>

// MIF
#include "mif/net/http/methods.h"
#include "mif/net/http/request_handler.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                class Session final
                    : public std::enable_shared_from_this<Session>
                {
                public:
                    struct Params final
                    {
                        boost::optional<std::size_t> headersSize;
                        boost::optional<std::size_t> bodySize;
                        boost::optional<std::chrono::microseconds> requestTimeout;

                        static std::size_t const DefaultPipelineLimit = 8;
                        std::size_t pipelineLimit = DefaultPipelineLimit;

                        Methods allowedMethods;

                        ServerHandlers handlers;
                    };

                    using ParamsPtr = std::shared_ptr<Params>;

                    Session(boost::asio::ip::tcp::socket &&socket, ParamsPtr params);

                    ~Session();

                    void Run();

                private:
                    boost::beast::tcp_stream m_stream;
                    ParamsPtr m_params;

                    class Queue;
                    std::unique_ptr<Queue> m_queue;

                    using BodyType = boost::beast::http::vector_body<char>;
                    using RequestParser = boost::beast::http::request_parser<BodyType>;

                    using BufferType = boost::beast::flat_buffer;

                    boost::optional<RequestParser> m_parser;
                    BufferType m_buffer;

                    void Read();
                    void OnRead(boost::beast::error_code ec, std::size_t bytes);

                    void OnWrite(bool close, boost::beast::error_code ec, std::size_t bytes);
                    void Close();

                    template <typename TBody, typename TAllocator>
                    using Request = boost::beast::http::request<TBody, boost::beast::http::basic_fields<TAllocator>>;

                    template <typename TBody, typename TAllocator>
                    void HandleRequest(Request<TBody, TAllocator> &&request);

                    void ReplyError(boost::beast::http::status status, std::string const &reason, bool isKeepAlive);

                    using ResponseBodyType = boost::beast::http::buffer_body;
                    using Response = boost::beast::http::response<ResponseBodyType>;

                    void Reply(Response &&response, bool isKeepAlive);
                };

            } // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_SESSION_H__