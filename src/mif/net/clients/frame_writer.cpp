// STD
#include <utility>

// MIF
#include "mif/net/clients/frame_writer.h"

namespace Mif
{
    namespace Net
    {
        namespace Clients
        {

            FrameWriter::FrameWriter(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher)
                : Client(control, publisher)
            {
            }

            void FrameWriter::ProcessData(Common::Buffer buffer)
            {
                Post(std::move(buffer));
            }

        }   // namespace Clients
    }   // namespace Net
}   // namespace Mif
