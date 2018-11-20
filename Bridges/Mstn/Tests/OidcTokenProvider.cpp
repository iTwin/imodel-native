/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/OidcTokenProvider.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include  <OidcNativeClient/OidcNative.h>
#include <WebServices/Configuration/UrlProvider.h>
#include "OidcTokenProvider.h"
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace OidcInterop;

#define IMODELHUB_ClientId            "imodel-bridge-framework-2664"
#define IMODELHUB_Scope               "openid imodelhub"

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
OidcTokenProvider::OidcTokenProvider(Credentials credentials):
    m_credentials(credentials)
    {}

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<ISecurityTokenPtr> OidcTokenProvider::UpdateToken()
    {
    auto token = OIDCNative::IssueToken(m_credentials.GetUsername().c_str(), m_credentials.GetPassword().c_str(), "http://localhost:32664/signin-oidc", IMODELHUB_ClientId, IMODELHUB_Scope);
    m_token = std::make_shared<OidcToken>(token);
    m_tokenValidUntil = BeTimePoint::FromNow(BeDuration::FromSeconds(60 * 55));

    return CreateCompletedAsyncTask<ISecurityTokenPtr>(m_token);
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ISecurityTokenPtr OidcTokenProvider::GetToken()
    {
    if (m_tokenValidUntil.IsInPast())
        m_token = nullptr;

    return m_token;
    }
