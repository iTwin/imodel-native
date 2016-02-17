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
#include <DgnClientFx/Utils/Http/AuthenticationHandler.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE IConnectAuthenticationProvider
    {
    public:
        virtual ~IConnectAuthenticationProvider()
            {};

        virtual std::shared_ptr<AuthenticationHandler> GetAuthenticationHandler
            (
            Utf8StringCR serverUrl,
            IHttpHandlerPtr customHandler = nullptr
            ) = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
