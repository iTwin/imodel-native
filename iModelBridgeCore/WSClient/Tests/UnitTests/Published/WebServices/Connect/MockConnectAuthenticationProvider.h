/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/MockConnectAuthenticationProvider.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <gmock/gmock.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

#if defined (USE_GTEST)
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockConnectAuthenticationProvider : public IConnectAuthenticationProvider
    {
    public:
        MOCK_CONST_METHOD2(GetAuthenticationHandler, AuthenticationHandlerPtr(Utf8StringCR, IHttpHandlerPtr));
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
