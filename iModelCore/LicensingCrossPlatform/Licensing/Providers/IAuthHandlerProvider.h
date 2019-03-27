/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Providers/IAuthHandlerProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <BeHttp/IHttpHandler.h>
#include <WebServices/Connect/IConnectAuthenticationProvider.h>

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES

BEGIN_BENTLEY_LICENSING_NAMESPACE

typedef std::shared_ptr<struct IAuthHandlerProvider> IAuthHandlerProviderPtr;

struct IAuthHandlerProvider
    {
public:
    virtual IHttpHandlerPtr GetAuthHandler(Utf8StringCR serverUrl, IConnectAuthenticationProvider::HeaderPrefix prefix) = 0;
    virtual ~IAuthHandlerProvider() {};
    };

END_BENTLEY_LICENSING_NAMESPACE
