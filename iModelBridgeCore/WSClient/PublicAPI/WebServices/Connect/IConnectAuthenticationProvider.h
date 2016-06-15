/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/IConnectAuthenticationProvider.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <BeHttp/AuthenticationHandler.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE IConnectAuthenticationProvider
    {
    public:
        virtual ~IConnectAuthenticationProvider()
            {};

        //! Get authentication handler for specific service server
        //! @param serverUrl should contain server URL without any directories
        //! @param httpHandler optional custom HTTP handler to send all requests trough
        virtual AuthenticationHandlerPtr GetAuthenticationHandler(Utf8StringCR serverUrl, IHttpHandlerPtr httpHandler = nullptr) = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
