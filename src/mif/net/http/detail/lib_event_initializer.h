//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     11.2016
//  Copyright (C) 2016-2020 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_DETAIL_LIB_EVENT_INITIALIZER_H__
#define __MIF_NET_HTTP_DETAIL_LIB_EVENT_INITIALIZER_H__

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Detail
            {

                class LibEventInitializer final
                {
                public:
                    LibEventInitializer(LibEventInitializer const &) = delete;
                    LibEventInitializer& operator = (LibEventInitializer const &) = delete;
                    LibEventInitializer(LibEventInitializer &&) = delete;
                    LibEventInitializer& operator = (LibEventInitializer &&) = delete;

                    static void Init();

                private:
                    LibEventInitializer();

                    static void OnFatal(int err);
                    static void LogMessage(int severity, char const *message);
                };

            }   // namespace Detail
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_DETAIL_LIB_EVENT_INITIALIZER_H__
