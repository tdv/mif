//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <stdexcept>

// THIS
#include "response.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                Response::Response(evhttp_request *request)
                    : m_request{request}
                {
                    if (!m_request)
                        throw std::invalid_argument{"[Mif::Net::Http::Detail::Response] Empty request pointer."};
                }

                void Response::Send()
                {
                    throw std::runtime_error{"[Mif::Net::Http::Detail::Response::Send] Not implemented."};
                }

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif
