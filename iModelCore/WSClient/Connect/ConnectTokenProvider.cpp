/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectTokenProvider.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectTokenProvider.h>

#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/Connect.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectTokenProvider::ConnectTokenProvider(std::shared_ptr<IConnectAuthenticationPersistence> customPersistence) :
m_persistence(customPersistence ? customPersistence : ConnectAuthenticationPersistence::GetShared())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr ConnectTokenProvider::UpdateToken()
    {
    Credentials creds = m_persistence->GetCredentials();

    auto token = std::make_shared<SamlToken>();
    if (!creds.IsValid() || 0 != Connect::Login(creds, *token))
        {
        return nullptr;
        }

    m_persistence->SetToken(token);
    return token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr ConnectTokenProvider::GetToken()
    {
    return m_persistence->GetToken();
    }
