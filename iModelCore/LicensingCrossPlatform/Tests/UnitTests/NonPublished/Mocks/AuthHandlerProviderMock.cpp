/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "AuthHandlerProviderMock.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

IHttpHandlerPtr AuthHandlerProviderMock::GetAuthHandler(Utf8StringCR serverUrl, IConnectAuthenticationProvider::HeaderPrefix prefix)
    {
    return m_mockedGetAuthHandler;
    }
