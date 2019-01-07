/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/Connect/IConnectTokenProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "ISecurityToken.h"
#include <Bentley/Tasks/AsyncTask.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct IConnectTokenProvider> IConnectTokenProviderPtr;
struct IConnectTokenProvider
    {
    virtual ~IConnectTokenProvider() {};

    //! Is used to get new token even if old one is valid.
    //! Should cache and return the token.
    //! Should return null token if it cannot be updated and authentication should fail.
    virtual AsyncTaskPtr<ISecurityTokenPtr> UpdateToken() = 0;

    //! Is used to get cached token for each request.
    //! Should return token if it is cached and valid, not expired.
    //! Should return null othervise. UpdateToken() will be called after null is returned.
    virtual ISecurityTokenPtr GetToken() = 0;
    };

END_BENTLEY_LICENSING_NAMESPACE
