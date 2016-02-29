/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/DelegationTokenProvider.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/DelegationTokenProvider.h>
#include "../Client/Logging.h"

#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ImsClient.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

#define TOKEN_LIFETIME 60

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DelegationTokenProvider::DelegationTokenProvider(IImsClientPtr client, Utf8String rpUri, IConnectTokenProviderPtr baseTokenProvider) :
m_client(client),
m_rpUri(rpUri),
m_baseTokenProvider(baseTokenProvider),
m_tokenLifetime(TOKEN_LIFETIME)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DelegationTokenProvider::Configure(uint64_t tokenLifetime)
    {
    m_tokenLifetime = tokenLifetime;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr DelegationTokenProvider::UpdateToken()
    {
    auto result = RetrieveNewToken()->GetResult();

    if (!result.IsSuccess())
        return nullptr;

    m_token = result.GetValue();
    return m_token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr DelegationTokenProvider::GetToken()
    {
    return m_token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SamlTokenResult> DelegationTokenProvider::RetrieveNewToken(bool updateBaseTokenIfFailed)
    {
    LOG.infov("Requesting '%s' delegation token", m_rpUri.c_str());
    SamlTokenPtr baseToken = m_baseTokenProvider->GetToken();
    if (nullptr == baseToken)
        {
        LOG.errorv("Base token not found for '%s' delegation token", m_rpUri.c_str());
        return CreateCompletedAsyncTask(SamlTokenResult::Error({}));
        }

    auto finalResult = std::make_shared<SamlTokenResult>();
    return m_client->RequestToken(*baseToken, m_rpUri, m_tokenLifetime)
    ->Then([=] (SamlTokenResult result)
        {
        *finalResult = result;

        if (result.IsSuccess())
            return;

        if (result.GetError().GetHttpStatus() != HttpStatus::Unauthorized)
            return;

        if (!updateBaseTokenIfFailed)
            return;

        if (nullptr == m_baseTokenProvider->UpdateToken())
            return;

        RetrieveNewToken(false)->Then([=] (SamlTokenResult result)
            {
            *finalResult = result;
            });
        })
    ->Then<SamlTokenResult>([=]
        {
        if (finalResult->IsSuccess())
            LOG.infov("Received '%s' delegation token lifetime %d minutes", m_rpUri.c_str(), finalResult->GetValue()->GetLifetime());
        return *finalResult;
        });
    }
