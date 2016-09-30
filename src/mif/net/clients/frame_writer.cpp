// STD
#include <algorithm>
#include <iterator>
#include <utility>

// BOOST
#include <boost/endian/conversion.hpp>

// MIF
#include "mif/net/clients/frame_writer.h"

namespace Mif
{
    namespace Net
    {
        namespace Clients
        {

            FrameWriter::FrameWriter(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher)
                : Client{control, publisher}
            {
            }

            void FrameWriter::ProcessData(Common::Buffer buffer)
            {
                auto frameBytes = static_cast<std::int32_t>(buffer.size());
                boost::endian::native_to_big_inplace(frameBytes);
                Common::Buffer frame;
                frame.reserve(buffer.size() + sizeof(frameBytes));
                std::copy(reinterpret_cast<char const *>(&frameBytes),
                    reinterpret_cast<char const *>(&frameBytes) + sizeof(frameBytes),
                    std::back_inserter(frame));
                frame.insert(std::end(frame), std::begin(buffer), std::end(buffer));
                Post(std::move(frame));
            }

        }   // namespace Clients
    }   // namespace Net
}   // namespace Mif
