/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <BeHttp/AuthenticationHandler.h>

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubAuthenticationHandler : public AuthenticationHandler
    {
    StubAuthenticationHandler (IHttpHandlerPtr defaultHttpHandler) :
        AuthenticationHandler (defaultHttpHandler)
        {}

    virtual Tasks::AsyncTaskPtr<AuthorizationResult> _RetrieveAuthorization (AttemptCR previousAttempt) 
        {
        return CreateCompletedAsyncTask(AuthorizationResult::Success("StubTestAuth"));
        }
    };