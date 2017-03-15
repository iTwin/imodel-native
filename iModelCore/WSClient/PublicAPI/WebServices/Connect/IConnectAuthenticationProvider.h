/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/IConnectAuthenticationProvider.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <MobileDgn/Utils/Http/AuthenticationHandler.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

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
        virtual AuthenticationHandlerPtr GetAuthenticationHandler(Utf8StringCR serverUrl, IHttpHandlerPtr httpHandler = nullptr) const = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
