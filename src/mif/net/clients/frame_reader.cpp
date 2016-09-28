// STD
#include <utility>

// MIF
#include "mif/net/clients/frame_reader.h"

namespace Mif
{
    namespace Net
    {
        namespace Clients
        {

            FrameReader::FrameReader(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher)
                : Client(control, publisher)
            {
            }

            void FrameReader::ProcessData(Common::Buffer buffer)
            {
                Post(std::move(buffer));
            }

        }   // namespace Clients
    }   // namespace Net
}   // namespace Mif
