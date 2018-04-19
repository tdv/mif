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
                            return {std::string{}};
                        }

                        Response Set()
                        {
                            return {std::string{}};
                        }

                        Response Find()
                        {
                            return {std::string{}};
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
