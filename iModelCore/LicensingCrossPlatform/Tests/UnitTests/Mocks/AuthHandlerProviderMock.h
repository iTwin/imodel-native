/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Mocks/AuthHandlerProviderMock.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include "../../../Licensing/Providers/IAuthHandlerProvider.h"

#include "../TestsHelper.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

BEGIN_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE

struct AuthHandlerProviderMock : IAuthHandlerProvider
    {
private:
    IHttpHandlerPtr m_mockedGetAuthHandler = nullptr;

public:
    IHttpHandlerPtr GetAuthHandler(Utf8StringCR serverUrl, IConnectAuthenticationProvider::HeaderPrefix prefix) override;
    void MockGetAuthHandler(IHttpHandlerPtr mocked) { m_mockedGetAuthHandler = mocked; };
    };

END_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE
