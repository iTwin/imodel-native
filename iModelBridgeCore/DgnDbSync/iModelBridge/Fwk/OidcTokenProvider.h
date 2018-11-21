/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/OidcTokenProvider.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Connect/IConnectTokenProvider.h>
#include "OidcToken.h"

BEGIN_BENTLEY_DGN_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct OidcTokenProvider : WebServices::IConnectTokenProvider
    {
    private:
        OidcTokenPtr m_token;

    public:
        OidcTokenProvider();
        Tasks::AsyncTaskPtr<WebServices::ISecurityTokenPtr> UpdateToken() override;
        WebServices::ISecurityTokenPtr GetToken() override;
    };
END_BENTLEY_DGN_NAMESPACE