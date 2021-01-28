//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2020
//  Copyright (C) 2016-2021 tdv
//-------------------------------------------------------------------

// STD
#include <deque>
#include <utility>

// BOOST
#include <boost/beast/websocket.hpp>
#include <boost/make_unique.hpp>

// MIF
#include "mif/common/log.h"
#include "mif/common/unused.h"
#include "mif/net/http/constants.h"

// THIS
#include "input_pack.h"
#include "session.h"
#include "utility.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                class Session::Queue final
                {
                public:
                    Queue(Session &session, std::size_t limit)
                        : m_session{session}
                        , m_limit{limit}
                    {
                    }

                    bool IsFull() const noexcept
                    {
                        return m_tasks.size() >= m_limit;
                    }

                    template <typename TBody, typename TFields>
                    using Response = boost::beast::http::response<TBody, TFields>;

                    void Post(OutputPackPtr out)
                    {
                        auto task = [this, out]
                        {
                            auto &resp = out->GetData();
                            boost::beast::http::async_write(m_session.m_stream, resp,
                                    boost::beast::bind_front_handler(&Session::OnWrite,
                                    m_session.shared_from_this(), resp.need_eof()));
                        };

                        m_tasks.emplace_back(std::move(task));

                        if (m_tasks.size() == 1)
                            m_tasks.front()();
                    }

                    bool Pop()
                    {
                        if (m_tasks.empty())
                        {
                            throw std::runtime_error{"[Mif::Net::Http::Detail::Session::Queue::Pop] "
                                    "Empty queue."};
                        }

                        auto const wasFull = IsFull();

                        m_tasks.pop_front();

                        if (!m_tasks.empty())
                            m_tasks.front()();

                        return wasFull;
                    }

                private:
                    Session &m_session;
                    std::size_t const m_limit;

                    std::deque<std::function<void ()>> m_tasks;
                };

                Session::Session(boost::asio::ip::tcp::socket &&socket, Params const &params)
                    : m_stream{std::move(socket)}
                    , m_params{params}
                    , m_queue{boost::make_unique<Queue>(*this,
                            std::max<std::size_t>(m_params.pipelineLimit, 1))
                        }
                {
                }

                Session::~Session()
                {
                }

                void Session::Run()
                {
                    boost::asio::dispatch(m_stream.get_executor(),
                            boost::beast::bind_front_handler(&Session::Read,
                            shared_from_this())
                        );
                }

                void Session::Read()
                {
                    m_parser.emplace();

                    m_parser->header_limit(m_params.headersSize);
                    m_parser->body_limit(m_params.bodySize);
                    m_stream.expires_after(m_params.requestTimeout);

                    boost::beast::http::async_read(m_stream, m_buffer, *m_parser,
                            boost::beast::bind_front_handler(&Session::OnRead,
                            shared_from_this())
                        );
                }

                void Session::OnRead(boost::beast::error_code ec, std::size_t bytes)
                {
                    Common::Unused(bytes);

                    if (ec == boost::beast::http::error::end_of_stream)
                    {
                        Close();
                        return;
                    }

                    if (ec)
                    {
                        MIF_LOG(Warning) << "[Mif::Net::Http::Detail::Session::OnRead] "
                                << "Failed to read data. Error: " << ec.message();

                        return;
                    }

                    if (boost::beast::websocket::is_upgrade(m_parser->get()))
                    {
                        MIF_LOG(Warning) << "[Mif::Net::Http::Detail::Session::OnRead] "
                                << "Failed to upgrade for starting websocket. Error: it has not implemented yet.";

                        Close();
                        return;
                    }

                    HandleRequest(m_parser->release());
                    // TODO:
                }

                void Session::OnWrite(bool close, boost::beast::error_code ec, std::size_t bytes)
                {
                    Common::Unused(bytes);

                    if (ec)
                    {
                        MIF_LOG(Info) << "[Mif::Net::Http::Detail::Session::OnWrite] "
                                << "Failed to write data. Error: " << ec.message();

                        return;
                    }

                    if (close)
                    {
                        Close();
                        return;
                    }

                    if (m_queue->Pop())
                        Read();
                }

                void Session::Close()
                {
                    boost::beast::error_code ec;
                    m_stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);

                    if (ec)
                    {
                        MIF_LOG(Warning) << "[Mif::Net::Http::Detail::Session::Close] "
                                << "Failed to close socket. Error: " << ec.message();
                    }
                }

                template <typename TBody, typename TAllocator>
                void Session::HandleRequest(Request<TBody, TAllocator> &&request)
                {
                    try
                    {
                        auto const &path = Utility::DecodeUrl(std::string{request.target()});

                        if (path.empty() || path[0] != '/' ||
                                path.find("..") != boost::beast::string_view::npos)
                        {
                            ReplyError(boost::beast::http::status::bad_request, "Bad path.", request.keep_alive());
                            return;
                        }

                        if (m_params.allowedMethods.find(
                                Utility::ConvertMethodType(request.method())) ==
                                std::end(m_params.allowedMethods))
                        {
                            ReplyError(boost::beast::http::status::method_not_allowed, "The method is not allowed.",
                                    request.keep_alive());
                            return;
                        }

                        if (m_params.handlers.empty())
                        {
                            ReplyError(boost::beast::http::status::method_not_allowed,
                                    "There is no any method for handling the request.",
                                    request.keep_alive());

                            return;
                        }

                        std::shared_ptr<IInputPack> in = std::make_shared<InputPack<Request<TBody, TAllocator>>>(request);
                        auto out = std::make_shared<OutputPack<Response>>();

                        auto process = [this, &in, &out] (std::string const &path)
                        {
                            auto iter = m_params.handlers.find(path);
                            if (iter != std::end(m_params.handlers))
                            {
                                iter->second(*in, *out);
                                return true;
                            }
                            return false;
                        };

                        auto handled = false;
                        std::string url{in->GetPath()};

                        if (process(url))
                            handled = true;
                        else
                        {
                            if (!url.empty() && url.back() != '/')
                                url.append("/");

                            for (auto pos = url.find_last_of('/') ; pos != std::string::npos ; pos = url.find_last_of('/'))
                            {
                                url = url.substr(0, pos);
                                auto const path = url.empty() ? std::string{"/"} : url;

                                if (process(path))
                                {
                                    handled = true;
                                    break;
                                }
                            }
                        }

                        if (!handled)
                        {
                            ReplyError(boost::beast::http::status::method_not_allowed,
                                    "Handler for the request processing was not found.",
                                    request.keep_alive());

                            return;
                        }

                        Reply(std::move(out), request.keep_alive());
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Detail::Session::HandleRequest] "
                                << "Failed to process request. Error: " << e.what();

                        ReplyError(boost::beast::http::status::internal_server_error, "An internal error has occurred.",
                                request.keep_alive());
                    }
                }

                void Session::ReplyError(boost::beast::http::status status, std::string const &reason,
                        bool isKeepAlive)
                {
                    auto out = std::make_shared<OutputPack<Response>>();

                    auto &iface = static_cast<IOutputPack &>(*out);

                    iface.SetCode(Detail::Utility::ConvertCode(status));
                    iface.SetReason(reason);
                    iface.SetData({std::begin(reason), std::end(reason)});
                    iface.SetHeader(Constants::Header::Response::ContentType::Value, "text/plain");
                    iface.SetHeader(Constants::Header::Response::Date::Value, Utility::CreateTimestamp());

                    auto &response = out->GetData();

                    //response.set(boost::beast::http::field::server, "Mif");
                    response.keep_alive(isKeepAlive);
                    response.version(11);

                    response.prepare_payload();

                    m_queue->Post(std::move(out));
                }

                void Session::Reply(OutputPackPtr out, bool isKeepAlive)
                {
                    auto &response = out->GetData();

                    //response.set(boost::beast::http::field::server, "Mif");
                    response.set(boost::beast::http::field::date, Utility::CreateTimestamp());
                    response.version(11);
                    response.keep_alive(isKeepAlive);

                    response.prepare_payload();

                    m_queue->Post(std::move(out));
                }

            } // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
