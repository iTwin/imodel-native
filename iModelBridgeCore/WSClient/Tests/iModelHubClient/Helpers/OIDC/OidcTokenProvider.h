/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../../Common.h"
#include "OidcToken.h"
#include <Bentley/BeTimeUtilities.h>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct OidcTokenProvider : IConnectTokenProvider
    {
    private:
        OidcTokenPtr m_token;
        BeTimePoint m_tokenValidUntil;
        Credentials m_credentials;

    public:
        OidcTokenProvider(Credentials credentials);
        AsyncTaskPtr<ISecurityTokenPtr> UpdateToken() override;
        ISecurityTokenPtr GetToken() override;
    };

END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
