/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Connect/IConnectTokenProvider.h>
#include "OidcToken.h"

BEGIN_BENTLEY_DGN_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Majerle                  12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct OidcTokenProvider : WebServices::IConnectTokenProvider
    {
    private:
        OidcTokenPtr m_token;

    public:
        OidcTokenProvider(Utf8String encodedToken);

        Tasks::AsyncTaskPtr<WebServices::ISecurityTokenPtr> UpdateToken() override;
        WebServices::ISecurityTokenPtr GetToken() override;
    };
END_BENTLEY_DGN_NAMESPACE