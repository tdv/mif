#ifndef __MIF_NET_CLIENT_FACTORY_H__
#define __MIF_NET_CLIENT_FACTORY_H__

// STD
#include <cstdint>
#include <memory>
#include <utility>

// MIF
#include "mif/common/index_sequence.h"
#include "mif/net/iclient_factory.h"

namespace Mif
{
    namespace Net
    {

        template <typename TClient>
        class ClientFactory final
            : public IClientFactory
        {
        public:
            template <typename ... TArgs>
            ClientFactory(TArgs && ... args)
                : m_impl{new Impl<TArgs ... >(std::forward<TArgs>(args) ... )}
            {
            }

        private:
            std::unique_ptr<IClientFactory> m_impl;

            template <typename ... TArgs>
            class Impl final
                : public IClientFactory
            {
            public:
                Impl(TArgs && ... args)
                    : m_args{std::make_tuple(std::forward<TArgs>(args) ... )}
                {
                }

            private:
                using TupleType = std::tuple<typename std::decay<TArgs>::type ... >;
                TupleType m_args;

                // IClientFactory
                virtual ClientPtr Create(IControlPtr control, IPublisherPtr publisher) override final
                {
                    return Create(control, publisher,
                        reinterpret_cast<Common::MakeIndexSequence<std::tuple_size<TupleType>::value> const *>(0));
                }

                template <std::size_t ... Indexes>
                ClientPtr Create(IControlPtr control, IPublisherPtr publisher,
                    Common::IndexSequence<Indexes ... > const *)
                {
                    return std::make_shared<TClient>(control, publisher, std::get<Indexes>(m_args) ... );
                }
            };

            // IClientFactory
            virtual ClientPtr Create(IControlPtr control, IPublisherPtr publisher) override final
            {
                return m_impl->Create(control, publisher);
            }
        };


    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_CLIENT_FACTORY_H__
