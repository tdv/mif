// STD
#include <utility>

// MIF
#include "mif/net/clients/gzip_compressor.h"

namespace Mif
{
    namespace Net
    {
        namespace Clients
        {

            GZipCompressor::GZipCompressor(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher)
                : Client(control, publisher)
            {
            }

            void GZipCompressor::ProcessData(Common::Buffer buffer)
            {
                Post(std::move(buffer));
            }

        }   // namespace Clients
    }   // namespace Net
}   // namespace Mif
