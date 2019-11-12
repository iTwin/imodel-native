/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#ifdef USE_GTEST
#include <gmock/gmock.h>
#include <BeHttp/AuthenticationHandler.h>

using namespace ::testing;
USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockAuthenticationHandler : public AuthenticationHandler
    {
    public:
        MockAuthenticationHandler (IHttpHandlerPtr defaultHttpHandler) :
            AuthenticationHandler (defaultHttpHandler)
            {
            }

        MOCK_METHOD1 (_RetrieveAuthorization, AsyncTaskPtr<AuthorizationResult> (AttemptCR previousAttempt));
    };
#endif