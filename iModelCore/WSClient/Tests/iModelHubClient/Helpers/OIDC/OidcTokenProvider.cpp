/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include  <OidcNativeClient/OidcNative.h>
#include "OidcSignInManager.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
using namespace OidcInterop;

#define IMODELHUB_ClientId            "imodel-hub-integration-tests-2485"
#define IMODELHUB_Scope               "openid profile email imodelhub"

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
    auto token = OIDCNative::IssueToken(m_credentials.GetUsername().c_str(), m_credentials.GetPassword().c_str(), UrlProvider::Urls::IMSOpenID.Get().c_str(), IMODELHUB_ClientId, IMODELHUB_Scope);
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
