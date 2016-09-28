// STD
#include <utility>

// MIF
#include "mif/net/clients/gzip_decompressor.h"

namespace Mif
{
    namespace Net
    {
        namespace Clients
        {

            GZipDecompressor::GZipDecompressor(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher)
                : Client(control, publisher)
            {
            }

            void GZipDecompressor::ProcessData(Common::Buffer buffer)
            {
                Post(std::move(buffer));
            }

        }   // namespace Clients
    }   // namespace Net
}   // namespace Mif
