/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/Connect/IConnectAuthenticationProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

//#include <WebServices/WebServices.h>
#include <Licensing/Licensing.h>
#include <BeHttp/AuthenticationHandler.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE
USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE IConnectAuthenticationProvider
    {
    public:
        //! Prefix to use in Authorization header
        enum class HeaderPrefix
            {
            Token,
            Saml,
            Bearer
            };

    public:
        virtual ~IConnectAuthenticationProvider()
            {};

        //! Get authentication handler for specific service server
        //! @param serverUrl should contain server URL without any directories
        //! @param httpHandler optional custom HTTP handler to send all requests trough
        //! @param prefix optional custom header prefix to use. Some services require different header format
        virtual AuthenticationHandlerPtr GetAuthenticationHandler
            (
            Utf8StringCR serverUrl,
            IHttpHandlerPtr httpHandler = nullptr,
            HeaderPrefix prefix = HeaderPrefix::Token
            ) const = 0;
    };

END_BENTLEY_LICENSING_NAMESPACE
