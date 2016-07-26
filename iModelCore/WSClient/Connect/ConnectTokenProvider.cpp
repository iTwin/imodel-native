/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectTokenProvider.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectTokenProvider.h>

#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ImsClient.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

#define TOKEN_LIFETIME (7*24*60)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectTokenProvider::ConnectTokenProvider(IImsClientPtr client, IConnectAuthenticationPersistencePtr customPersistence) :
m_client(client),
m_persistence(customPersistence ? customPersistence : ConnectAuthenticationPersistence::GetShared()),
m_tokenLifetime(TOKEN_LIFETIME)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectTokenProvider::Configure(uint64_t tokenLifetime)
    {
    m_tokenLifetime = tokenLifetime;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr ConnectTokenProvider::UpdateToken()
    {
    Credentials creds = m_persistence->GetCredentials();
    if (!creds.IsValid())
        return nullptr;

    auto rpUri = ImsClient::GetLegacyRelyingPartyUri();
    auto result = m_client->RequestToken(creds, rpUri, m_tokenLifetime)->GetResult();
    if (!result.IsSuccess())
        return nullptr;

    SamlTokenPtr token = result.GetValue();

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
