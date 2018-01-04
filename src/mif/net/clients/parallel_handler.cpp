//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     02.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// MIF
#include "mif/common/log.h"
#include "mif/net/clients/parallel_handler.h"

namespace Mif
{
    namespace Net
    {
        namespace Clients
        {

            class ParallelHandler::Impl final
                : public std::enable_shared_from_this<Impl>
            {
            public:
                Impl(ParallelHandler &owner, Common::IThreadPoolPtr workers)
                    : m_owner{owner}
                    , m_workers{std::move(workers)}
                {
                }

                void OnData(Common::Buffer buffer)
                {
                    auto self = shared_from_this();
                    auto data = std::make_shared<Common::Buffer>(std::move(buffer));
                    m_workers->Post([self, data] ()
                            {
                                try
                                {
                                    Common::Buffer buffer;
                                    std::swap(buffer, *data);
                                    self->m_owner.Post(std::move(buffer));
                                }
                                catch (std::exception const &e)
                                {
                                    self->m_owner.CloseMe();
                                    MIF_LOG(Warning) << "[Mif::Net::Clients::ParallelHandler::Impl::OnData] "
                                            << "Failed to process data. Error: " << e.what();
                                }
                            }
                        );
                }

            private:
                ParallelHandler &m_owner;
                Common::IThreadPoolPtr m_workers;
            };

            ParallelHandler::ParallelHandler(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher,
                    Common::IThreadPoolPtr workers)
                : Client{control, publisher}
                , m_impl{std::make_shared<Impl>(*this, std::move(workers))}
            {
            }

            ParallelHandler::~ParallelHandler()
            {
            }

            void ParallelHandler::ProcessData(Common::Buffer buffer)
            {
                m_impl->OnData(std::move(buffer));
            }

        }   // namespace Clients
    }   // namespace Net
}   // namespace Mif
