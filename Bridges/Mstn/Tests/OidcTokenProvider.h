/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/OidcTokenProvider.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/WebServices.h>
#include <WebServices/Connect/IConnectTokenProvider.h>
#include "OidcToken.h"
#include <Bentley/BeTimeUtilities.h>
#include <BeHttp/Credentials.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct OidcTokenProvider : WebServices::IConnectTokenProvider
    {
    private:
        OidcTokenPtr m_token;
        BeTimePoint m_tokenValidUntil;
        Http::Credentials m_credentials;

    public:
        OidcTokenProvider(Http::Credentials credentials);
        AsyncTaskPtr<ISecurityTokenPtr> UpdateToken() override;
        ISecurityTokenPtr GetToken() override;
    };

