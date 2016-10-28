//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_CLIENTS_CHAIN_H__
#define __MIF_NET_CLIENTS_CHAIN_H__

// STD
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>

// MIF
#include "mif/common/index_sequence.h"
#include "mif/common/creator.h"

namespace Mif
{
    namespace Net
    {

    template <typename ... TClients>
    class ClientsChain final
        : public Client
    {
    public:
        template <typename ... TCreators>
        ClientsChain(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher,
                     TCreators && ... creators)
            : Client{control, publisher}
            , m_client{CreateChain<TClients ... >(std::make_shared<ChainClient>(*this),
                std::forward<TCreators>(creators) ... )}
        {
        }

        template <typename TClient>
        typename std::enable_if<std::is_base_of<Client, TClient>::value, std::shared_ptr<TClient>>::type
        GetClientItem()
        {
            auto *item = dynamic_cast<TClient *>(m_client->GetClient(typeid(TClient)));
            if (!item)
                throw std::bad_cast{};
            return std::shared_ptr<TClient>(this->shared_from_this(), item);
        }

    private:
        using ThisType = ClientsChain<TClients ... >;

        struct IHolder
            : public std::enable_shared_from_this<IHolder>
            , public IHandler
            , public IPublisher
            , public IControl
        {
            virtual ~IHolder() = default;
            virtual Client* GetClient(std::type_info const &info) = 0;
        };

        std::shared_ptr<IHolder> m_client;

        // Client
        virtual void ProcessData(Common::Buffer buffer) override final
        {
            m_client->OnData(std::move(buffer));
        }

        template <typename ... T, typename ... TCreators>
        typename std::enable_if<sizeof ... (T) == 1, std::shared_ptr<IHolder>>::type
        CreateChain(std::shared_ptr<IHolder> prev, TCreators && ... creators)
        {
            using TClient = typename std::tuple_element<0, std::tuple<T ... >>::type;
            auto holder = std::make_shared<Holder<TClient>>(prev);
            holder->Init(std::forward<TCreators>(creators) ... );
            return holder;
        }

        template <typename T, typename ... L, typename ... TCreators>
        typename std::enable_if<sizeof ... (L) != 0, std::shared_ptr<IHolder>>::type
        CreateChain(std::shared_ptr<IHolder> prev, TCreators && ... creators)
        {
            auto next = CreateChain<L ... >(prev, std::forward<TCreators>(creators) ... );
            auto holder = std::make_shared<Holder<T>>(next);
            holder->Init(std::forward<TCreators>(creators) ... );
            return holder;
        }

        template <typename ... T, typename ... TCreators>
        typename std::enable_if<sizeof ... (T) == 0, std::shared_ptr<IHolder>>::type
        CreateChain(std::shared_ptr<IHolder> prev, TCreators && ... )
        {
            return prev;
        }

        class ChainClient final
            : public IHolder
        {
        public:
            ChainClient(ThisType &owner)
                : m_owner{owner}
            {
            }

        private:
            ThisType &m_owner;

            // IHolder
            virtual Client* GetClient(std::type_info const &info) override final
            {
                (void)info;
                throw std::runtime_error{"[Mif::Net::ClientsChain::ChainClient::GetClient] Client item not found."};
            }

            // IHandler
            virtual void OnData(Common::Buffer buffer) override final
            {
                m_owner.Post(std::move(buffer));
            }

            // IPublisher
            virtual void Publish(Common::Buffer buffer) override final
            {
                (void)buffer;
            }

            // IControl
            virtual void CloseMe() override final
            {
                m_owner.CloseMe();
            }
        };

        template <typename TClient>
        class Holder final
            : public IHolder
        {
        public:
            Holder(std::shared_ptr<IHolder> next)
                : m_next{next}
            {
            }

            template <typename ... TCreators>
            void Init(TCreators && ... creators)
            {
                m_client = CreateClient(std::forward<TCreators>(creators) ... );
            }

        private:
            std::shared_ptr<IHolder> m_next;
            using TClientPtr = std::shared_ptr<TClient>;
            TClientPtr m_client;

            template <typename ... TCreators>
            typename std::enable_if<sizeof ... (TCreators) == 0, TClientPtr>::type
            CreateClient(TCreators && ... /*creators*/)
            {
                return std::make_shared<TClient>(
                        std::dynamic_pointer_cast<IControl>(this->shared_from_this()),
                        std::dynamic_pointer_cast<IPublisher>(this->shared_from_this())
                    );
            }

            template <typename TCreator, typename ... TCreators>
            TClientPtr CreateClient(TCreator && creator, TCreators && ... creators)
            {
                return Create(std::forward<TCreator>(creator), std::forward<TCreators>(creators) ... );
            }

            template <typename TCreator, typename ... TCreators>
            typename std::enable_if<std::is_same<TClient, typename TCreator::Type>::value, TClientPtr>::type
            Create(TCreator creator, TCreators && ... /*creators*/)
            {
                return creator.Create(
                        std::dynamic_pointer_cast<IControl>(this->shared_from_this()),
                        std::dynamic_pointer_cast<IPublisher>(this->shared_from_this())
                    );
            }

            template <typename TCreator, typename ... TCreators>
            typename std::enable_if<!std::is_same<TClient, typename TCreator::Type>::value, TClientPtr>::type
            Create(TCreator /*creator*/, TCreators && ... creators)
            {
                return CreateClient(std::forward<TCreators>(creators) ... );
            }

            // IHandler
            virtual void OnData(Common::Buffer buffer) override final
            {
                m_client->OnData(std::move(buffer));
            }

            // IHolder
            virtual Client* GetClient(std::type_info const &info) override final
            {
                return typeid(TClient) == info ? m_client.get() : m_next->GetClient(info);
            }

            // IPublisher
            virtual void Publish(Common::Buffer buffer) override final
            {
                m_next->OnData(std::move(buffer));
            }

            // IControl
            virtual void CloseMe() override final
            {
                m_next->CloseMe();
            }
        };
    };

    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_CLIENTS_CHAIN_H__
