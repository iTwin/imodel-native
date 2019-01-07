/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Connect/ConnectTokenProvider.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#include "ClientInternal.h"
#include "../../PublicAPI/Licensing/Connect/ConnectTokenProvider.h"

#include "../../PublicAPI/Licensing/Connect/ConnectAuthenticationPersistence.h"
#include "../../PublicAPI/Licensing/Connect/ImsClient.h"

USING_NAMESPACE_BENTLEY_LICENSING

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
AsyncTaskPtr<ISecurityTokenPtr> ConnectTokenProvider::UpdateToken()
    {
    Credentials creds = m_persistence->GetCredentials();
    if (!creds.IsValid())
        return CreateCompletedAsyncTask(ISecurityTokenPtr());

    auto rpUri = ImsClient::GetLegacyRelyingPartyUri();
    return  m_client->RequestToken(creds, rpUri, m_tokenLifetime)
        ->Then<ISecurityTokenPtr>([=] (SamlTokenResult result)
        {
        if (!result.IsSuccess())
            return SamlTokenPtr();

        SamlTokenPtr token = result.GetValue();
        m_persistence->SetToken(token);
        return token;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ISecurityTokenPtr ConnectTokenProvider::GetToken()
    {
    return m_persistence->GetToken();
    }
