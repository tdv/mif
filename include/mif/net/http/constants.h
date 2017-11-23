//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     12.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_NET_HTTP_CONSTANTS_H__
#define __MIF_NET_HTTP_CONSTANTS_H__

// MIF
#include "mif/common/static_string.h"

namespace Mif
{
    namespace Net
    {
        namespace Http
        {
            namespace Constants
            {
                namespace Header
                {

                    namespace MifExt
                    {

                        using Session = MIF_STATIC_STR("X-Mif-Session");

                    }   // namespace MifExt

                    namespace Request
                    {

                        using Accept = MIF_STATIC_STR("Accept");
                        using AcceptCharset = MIF_STATIC_STR("Accept-Charset");
                        using AcceptEncoding = MIF_STATIC_STR("Accept-Encoding");
                        using AcceptLanguage = MIF_STATIC_STR("Accept-Language");
                        using AcceptDatetime = MIF_STATIC_STR("Accept-Datetime");
                        using AccessControlRequestMethod = MIF_STATIC_STR("Access-Control-Request-Method");
                        using AccessControlRequestHeaders = MIF_STATIC_STR("Access-Control-Request-Headers");
                        using Authorization = MIF_STATIC_STR("Authorization");
                        using CacheControl = MIF_STATIC_STR("Cache-Control");
                        using Connection = MIF_STATIC_STR("Connection");
                        using Cookie = MIF_STATIC_STR("Cookie");
                        using ContentLength = MIF_STATIC_STR("Content-Length");
                        using ContentMD5 = MIF_STATIC_STR("Content-MD5");
                        using ContentType = MIF_STATIC_STR("Content-Type");
                        using Date = MIF_STATIC_STR("Date");
                        using Expect = MIF_STATIC_STR("Expect");
                        using Forwarded = MIF_STATIC_STR("Forwarded");
                        using From = MIF_STATIC_STR("From");
                        using Host = MIF_STATIC_STR("Host");
                        using IfMatch = MIF_STATIC_STR("If-Match");
                        using IfModifiedSince = MIF_STATIC_STR("If-Modified-Since");
                        using IfNoneMatch = MIF_STATIC_STR("If-None-Match");
                        using IfRange = MIF_STATIC_STR("If-Range");
                        using IfUnmodifiedSince = MIF_STATIC_STR("If-Unmodified-Since");
                        using MaxForwards = MIF_STATIC_STR("Max-Forwards");
                        using Origin = MIF_STATIC_STR("Origin");
                        using Pragma = MIF_STATIC_STR("Pragma");
                        using ProxyAuthorization = MIF_STATIC_STR("Proxy-Authorization");
                        using Range = MIF_STATIC_STR("Range");
                        using Referer = MIF_STATIC_STR("Referer");
                        using TE = MIF_STATIC_STR("TE");
                        using UserAgent = MIF_STATIC_STR("User-Agent");
                        using Upgrade = MIF_STATIC_STR("Upgrade");
                        using Via = MIF_STATIC_STR("Via");
                        using Warning = MIF_STATIC_STR("Warning");

                    }   // namespace Request

                    namespace Response
                    {

                        using AccessControlAllowOrigin = MIF_STATIC_STR("Access-Control-Allow-Origin");
                        using AccessControlAllowCredentials = MIF_STATIC_STR("Access-Control-Allow-Credentials");
                        using AccessControlExposeHeaders = MIF_STATIC_STR("Access-Control-Expose-Headers");
                        using AccessControlMaxAge = MIF_STATIC_STR("Access-Control-Max-Age");
                        using AccessControlAllowMethods = MIF_STATIC_STR("Access-Control-Allow-Methods");
                        using AccessControlAllowHeaders = MIF_STATIC_STR("Access-Control-Allow-Headers");
                        using AcceptPatch = MIF_STATIC_STR("Accept-Patch");
                        using AcceptRanges = MIF_STATIC_STR("Accept-Ranges");
                        using Age = MIF_STATIC_STR("Age");
                        using Allow = MIF_STATIC_STR("Allow");
                        using AltSvc = MIF_STATIC_STR("Alt-Svc");
                        using CacheControl = MIF_STATIC_STR("Cache-Control");
                        using Connection = MIF_STATIC_STR("Connection");
                        using ContentDisposition = MIF_STATIC_STR("Content-Disposition");
                        using ContentEncoding = MIF_STATIC_STR("Content-Encoding");
                        using ContentLanguage = MIF_STATIC_STR("Content-Language");
                        using ContentLength = MIF_STATIC_STR("Content-Length");
                        using ContentLocation = MIF_STATIC_STR("Content-Location");
                        using ContentMD5 = MIF_STATIC_STR("Content-MD5");
                        using ContentRange = MIF_STATIC_STR("Content-Range");
                        using ContentType = MIF_STATIC_STR("Content-Type");
                        using Date = MIF_STATIC_STR("Date");
                        using ETag = MIF_STATIC_STR("ETag");
                        using Expires = MIF_STATIC_STR("Expires");
                        using LastModified = MIF_STATIC_STR("Last-Modified");
                        using Link = MIF_STATIC_STR("Link");
                        using Location = MIF_STATIC_STR("Location");
                        using P3P = MIF_STATIC_STR("P3P");
                        using Pragma = MIF_STATIC_STR("Pragma");
                        using ProxyAuthenticate = MIF_STATIC_STR("Proxy-Authenticate");
                        using PublicKeyPins = MIF_STATIC_STR("Public-Key-Pins");
                        using RetryAfter = MIF_STATIC_STR("Retry-After");
                        using Server = MIF_STATIC_STR("Server");
                        using SetCookie = MIF_STATIC_STR("Set-Cookie");
                        using StrictTransportSecurity = MIF_STATIC_STR("Strict-Transport-Security");
                        using Trailer = MIF_STATIC_STR("Trailer");
                        using TransferEncoding = MIF_STATIC_STR("Transfer-Encoding");
                        using Tk = MIF_STATIC_STR("Tk");
                        using Upgrade = MIF_STATIC_STR("Upgrade");
                        using Vary = MIF_STATIC_STR("Vary");
                        using Via = MIF_STATIC_STR("Via");
                        using Warning = MIF_STATIC_STR("Warning");
                        using WWWAuthenticate = MIF_STATIC_STR("WWW-Authenticate");
                        using XFrameOptions = MIF_STATIC_STR("X-Frame-Options");

                    }   //namespace Response
                }   // namespace Header

                namespace Value
                {
                    namespace Connection
                    {

                        using KeepAlive = MIF_STATIC_STR("keep-alive");
                        using Close = MIF_STATIC_STR("close");

                    }   // namespace Connection
                }   // namespace Value
            }   // namespace Constants
        }   // namespace Http
    }   // namespace Net
}   // namespace Mif

#endif  // !__MIF_NET_HTTP_CONSTANTS_H__
