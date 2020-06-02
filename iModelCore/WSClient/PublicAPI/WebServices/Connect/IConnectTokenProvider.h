/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/ISecurityToken.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/AsyncResult.h>
#include <Bentley/Tasks/AsyncError.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct IConnectTokenProvider> IConnectTokenProviderPtr;
typedef AsyncResult<ISecurityTokenPtr, AsyncError> ISecurityTokenResult;

struct IConnectTokenProvider
    {
    virtual ~IConnectTokenProvider() {};

    //! Is used to get new token even if old one is valid.
    //! If successful should cache and return the token.
    //! If cannot be updated should return error.
    virtual AsyncTaskPtr<ISecurityTokenResult> UpdateTokenWithResult()
        {
        return UpdateToken()
            ->Then<ISecurityTokenResult>([=] (ISecurityTokenPtr token)
            {
            if (nullptr == token)
                return ISecurityTokenResult::Error(AsyncError());

            return ISecurityTokenResult::Success(token);
            });
        };

    //! Is used to get new token even if old one is valid.
    //! Should cache and return the token.
    //! Should return null token if it cannot be updated and authentication should fail.
    virtual AsyncTaskPtr<ISecurityTokenPtr> UpdateToken() = 0;

    //! Is used to get cached token for each request.
    //! Should return token if it is cached and valid, not expired.
    //! Should return null othervise. UpdateToken() will be called after null is returned.
    virtual ISecurityTokenPtr GetToken() = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
