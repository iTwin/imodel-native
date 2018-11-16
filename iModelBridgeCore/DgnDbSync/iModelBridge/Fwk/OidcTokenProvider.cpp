/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/OidcTokenProvider.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "OidcTokenProvider.h"


#define IMODELHUB_ClientId            "imodel-hub-integration-tests-2485"
#define IMODELHUB_Scope               "openid profile email imodelhub"

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
OidcTokenProvider::OidcTokenProvider()
    {}

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Tasks::AsyncTaskPtr<WebServices::ISecurityTokenPtr> OidcTokenProvider::UpdateToken()
    {
    m_token = std::make_shared<OidcToken>("");
    return Tasks::CreateCompletedAsyncTask<WebServices::ISecurityTokenPtr>(m_token);
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WebServices::ISecurityTokenPtr OidcTokenProvider::GetToken()
    {
    //Is token valid ?

    return m_token;
    }
