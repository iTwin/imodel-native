/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/MockAuthenticationHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#ifdef USE_GTEST
#include <gmock/gmock.h>
#else
#include <Bentley/BeTest.h>
#endif

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

#ifdef USE_GTEST
        MOCK_METHOD1 (_RetrieveAuthorization, AsyncTaskPtr<AuthorizationResult> (AttemptCR previousAttempt));
#endif
    };