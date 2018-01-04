//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// STD
#include <algorithm>
#include <iterator>
#include <utility>

// BOOST
#include <boost/endian/conversion.hpp>

// MIF
#include "mif/net/clients/frame_reader.h"

namespace Mif
{
    namespace Net
    {
        namespace Clients
        {

            class FrameReader::Impl final
            {
            public:
                void OnData(Common::Buffer buffer, FrameReader &owner)
                {
                    if (!m_buffer.size())
                        m_buffer.swap(buffer);
                    else
                        m_buffer.insert(std::end(m_buffer), std::begin(buffer), std::end(buffer));
                    std::int32_t frameBytes = 0;
                    if (m_buffer.size() < sizeof(frameBytes))
                        return;
                    frameBytes = *reinterpret_cast<decltype(frameBytes) const *>(m_buffer.data());
                    boost::endian::big_to_native_inplace(frameBytes);
                    if (m_buffer.size() < frameBytes + sizeof(frameBytes))
                        return;
                    Common::Buffer{}.swap(buffer);
                    auto begin = std::begin(m_buffer);
                    std::advance(begin, sizeof(frameBytes));
                    auto end = begin;
                    std::advance(end, frameBytes);
                    buffer.assign(begin, end);
                    if (end == std::end(m_buffer))
                    {
                        Common::Buffer{}.swap(m_buffer);
                    }
                    else
                    {
                        Common::Buffer newBuffer{end, std::end(m_buffer)};
                        m_buffer.swap(newBuffer);
                    }
                    owner.Post(std::move(buffer));
                }

            private:
                Common::Buffer m_buffer;
            };

            FrameReader::FrameReader(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher)
                : Client{control, publisher}
                , m_impl{new Impl{}}
            {
            }

            FrameReader::~FrameReader()
            {
            }

            void FrameReader::ProcessData(Common::Buffer buffer)
            {
                m_impl->OnData(std::move(buffer), *this);
            }

        }   // namespace Clients
    }   // namespace Net
}   // namespace Mif
