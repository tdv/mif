#ifndef __PHONEBOOK_HANDLER_BASE_H__
#define __PHONEBOOK_HANDLER_BASE_H__

// MIF
#include <mif/net/http/converter/content/json.h>
#include <mif/net/http/serializer/json.h>
#include <mif/net/http/web_service.h>

// THIS
#include "data/data.h"

namespace Phonebook
{
    namespace Service
    {
        namespace Handler
        {

            class Base
                : public Mif::Net::Http::WebService
            {
            protected:
                template <typename T>
                using RequestContent = Content<T, Mif::Net::Http::Converter::Content::Json>;

                using Response = Result<Mif::Net::Http::Serializer::Json>;

                Base(std::string const &prefix);

                template <typename C, typename R, typename ... Args>
                void AddHandler(std::string const &path, C *object, R (C::*handler)(Args ... ))
                {
                    WebService::AddHandler(m_prefix + path, object, handler);
                }

                Data::Info GetMeta(std::int32_t code = 0, std::string message = {}) const;

            private:
                std::string m_prefix;

                // WebService
                virtual void OnException(Mif::Net::Http::IInputPack const &request,
                        Mif::Net::Http::IOutputPack &response, std::exception_ptr exception,
                        Mif::Net::Http::Code &code, std::string &message) const override final;
            };

        }   // namespace Handler
    }   // namespace Service
}   // namespace Phonebook

#endif  // !__PHONEBOOK_HANDLER_BASE_H__
