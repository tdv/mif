// MIF
#include <mif/service/creator.h>
#include <mif/service/root_locator.h>

// THIS
#include "base.h"
#include "data/meta/data.h"
#include "id/service.h"
#include "interface/istorage.h"

namespace Phonebook
{
    namespace Service
    {
        namespace Handler
        {
            namespace Detail
            {
                namespace
                {

                    class Phonebook
                        : public Base
                    {
                    public:
                        Phonebook(std::string const &prefix)
                            : Base{prefix}
                        {
                            AddHandler("/list", this, &Phonebook::List);
                            AddHandler("/set", this, &Phonebook::Set);
                            AddHandler("/find", this, &Phonebook::Find);
                        }

                    private:
                        // WebService.Hadlers
                        Response List()
                        {
                            auto storage = Mif::Service::RootLocator::Get()->Get<IStorage>(Id::Storage);

                            Data::Phones response;

                            response.meta = GetMeta();
                            response.data = storage->List();

                            return response;
                        }

                        Response Set(Prm<std::string, Name("phone")> const &phone,
                                     Prm<std::string, Name("name")> const &name)
                        {
                            if (!phone)
                                throw std::invalid_argument{"No \"phone\" parameter."};
                            if (phone.Get().empty())
                                throw std::invalid_argument{"\"phone\" must not be empty."};

                            if (!name)
                                throw std::invalid_argument{"No \"name\" parameter."};
                            if (name.Get().empty())
                                throw std::invalid_argument{"\"name\" must not be empty."};

                            auto storage = Mif::Service::RootLocator::Get()->Get<IStorage>(Id::Storage);
                            storage->Set(phone.Get(), name.Get());

                            Data::Header response;
                            response.meta = GetMeta();
                            return response;
                        }

                        Response Find(Prm<std::string, Name("name")> const &name)
                        {
                            if (!name)
                                throw std::invalid_argument{"No \"name\" parameter."};
                            if (name.Get().empty())
                                throw std::invalid_argument{"\"name\" must not be empty."};

                            auto storage = Mif::Service::RootLocator::Get()->Get<IStorage>(Id::Storage);

                            Data::Phones response;

                            response.meta = GetMeta();
                            response.data = storage->Find(name.Get());

                            return response;
                        }
                    };

                }   // namespace
            }   // namespace Detail
        }   // namespace Handler
    }   // namespace Service
}   // namespace Phonebook

MIF_SERVICE_CREATOR
(
    ::Phonebook::Id::Phonebook,
    ::Phonebook::Service::Handler::Detail::Phonebook,
    std::string
)
