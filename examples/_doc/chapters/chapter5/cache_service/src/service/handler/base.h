#ifndef __CACHESERVICE_HANDLER_BASE_H__
#define __CACHESERVICE_HANDLER_BASE_H__

// MIF
#include <mif/net/http/serializer/json.h>
#include <mif/net/http/web_service.h>

namespace CacheService
{
    namespace Handler
    {

        class Base
            : public Mif::Net::Http::WebService
        {
        protected:
            using Response = Result<Mif::Net::Http::Serializer::Json>;

            Base(std::string const &prefix);

            template <typename C, typename R, typename ... Args>
            void AddHandler(std::string const &path, C *object, R (C::*handler)(Args ... ))
            {
                WebService::AddHandler(m_prefix + path, object, handler);
            }

        private:
            std::string m_prefix;
        };

    }   // namespace Handler
}   // namespace CacheService

#endif  // !__CACHESERVICE_HANDLER_BASE_H__
