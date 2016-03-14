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

#define TOKEN_REQUEST_LIFETIME 60
#define TOKEN_EXPIRATION_THRESHOLD 5

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DelegationTokenProvider::DelegationTokenProvider(IImsClientPtr client, Utf8String rpUri, IConnectTokenProviderPtr parentTokenProvider) :
m_client(client),
m_rpUri(rpUri),
m_parentTokenProvider(parentTokenProvider),
m_tokenLifetime(0),
m_tokenRequestLifetime(TOKEN_REQUEST_LIFETIME),
m_tokenExpirationThreshold(TOKEN_EXPIRATION_THRESHOLD)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DelegationTokenProvider::Configure(uint32_t tokenLifetime, uint32_t tokenExpirationThreshold)
    {
    m_tokenRequestLifetime = tokenLifetime;
    m_tokenExpirationThreshold = tokenExpirationThreshold;
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
    m_tokenLifetime = m_token->GetLifetime();
    m_tokenUpdateDate = DateTime::GetCurrentTimeUtc();

    return m_token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr DelegationTokenProvider::GetToken()
    {
    ValidateToken();
    return m_token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DelegationTokenProvider::ValidateToken()
    {
    if (nullptr == m_token)
        return;

    int64_t tokenUpdateMs = 0;
    int64_t currentMs = 0;
    m_tokenUpdateDate.ToUnixMilliseconds(tokenUpdateMs);
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentMs);

    if (currentMs < (tokenUpdateMs + m_tokenLifetime * 60000 - m_tokenExpirationThreshold * 60000))
        return;

    m_token = nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SamlTokenResult> DelegationTokenProvider::RetrieveNewToken(bool updateBaseTokenIfFailed)
    {
    LOG.infov("Requesting '%s' delegation token", m_rpUri.c_str());
    SamlTokenPtr parentToken = m_parentTokenProvider->GetToken();
    if (nullptr == parentToken)
        {
        LOG.errorv("Base token not found for '%s' delegation token", m_rpUri.c_str());
        return CreateCompletedAsyncTask(SamlTokenResult::Error({}));
        }

    auto finalResult = std::make_shared<SamlTokenResult>();
    return m_client->RequestToken(*parentToken, m_rpUri, m_tokenRequestLifetime)
        ->Then([=] (SamlTokenResult result)
        {
        *finalResult = result;

        if (result.IsSuccess())
            return;

        if (result.GetError().GetHttpStatus() != HttpStatus::Unauthorized)
            return;

        if (!updateBaseTokenIfFailed)
            return;

        if (nullptr == m_parentTokenProvider->UpdateToken())
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
