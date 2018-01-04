//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// MIF
#include "mif/common/log.h"

// THIS
#include "server_thread.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                ServerThread::ServerThread(std::string const &host, std::string const &port,
                        ServerHandler const &handler, Methods const &allowedMethods,
                        std::size_t headersSize, std::size_t bodySize, std::size_t requestTimeout)
                {
                    m_thread.reset(new std::thread{[this, &host, &port, &handler, &allowedMethods, &headersSize, &bodySize, &requestTimeout]()
                            {
                                try
                                {
                                    m_server.reset(new Server{host, port, handler, allowedMethods, headersSize, bodySize, requestTimeout});
                                    m_server->Run();
                                }
                                catch (...)
                                {
                                    m_exception = std::current_exception();
                                }
                            }
                        });

                    while (!m_exception && !m_server)
                        std::this_thread::sleep_for(m_timeout);

                    if (m_exception)
                        std::rethrow_exception(m_exception);
                }

                ServerThread::ServerThread(evutil_socket_t socket, ServerHandler const &handler, Methods const &allowedMethods,
                        std::size_t headersSize, std::size_t bodySize, std::size_t requestTimeout)
                {
                    m_thread.reset(new std::thread{[this, &socket, &handler, &allowedMethods, &headersSize, &bodySize, &requestTimeout]()
                            {
                                try
                                {
                                    m_server.reset(new Server{socket, handler, allowedMethods, headersSize, bodySize, requestTimeout});
                                    m_server->Run();
                                }
                                catch (...)
                                {
                                    m_exception = std::current_exception();
                                }
                            }
                        });

                    while (!m_exception && !m_server)
                        std::this_thread::sleep_for(m_timeout);

                    if (m_exception)
                        std::rethrow_exception(m_exception);
                }

                ServerThread::~ServerThread()
                {
                    if (m_server)
                        m_server->Stop();
                }

                evutil_socket_t ServerThread::GetSocket() const
                {
                    return m_server ? m_server->GetSocket() : -1;
                }

                void ServerThread::ThreadDeleter(std::thread *thread)
                {
                    if (!thread)
                        return;
                    try
                    {
                        thread->join();
                    }
                    catch (std::exception const &e)
                    {
                        MIF_LOG(Error) << "[Mif::Net::Http::Detail::ServerThread::ThreadDeleter] "
                            << "Failed to join server thread.";
                    }

                    delete thread;
                }

            } // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
