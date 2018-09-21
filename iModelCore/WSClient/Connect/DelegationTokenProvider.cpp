/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/DelegationTokenProvider.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "../Client/Logging.h"
#include "DelegationTokenProvider.h"

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
AsyncTaskPtr<ISecurityTokenPtr> DelegationTokenProvider::UpdateToken()
    {
    return RetrieveNewToken()->Then<ISecurityTokenPtr>([=] (SamlTokenResult result)
        {
        if (!result.IsSuccess())
            return SamlTokenPtr();

        m_token = result.GetValue();
        m_tokenLifetime = m_token->GetLifetime();
        m_tokenUpdateDate = DateTime::GetCurrentTimeUtc();

        return m_token;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ISecurityTokenPtr DelegationTokenProvider::GetToken()
    {
    ValidateToken();
    return m_token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                      
+---------------+---------------+---------------+---------------+---------------+------*/
void DelegationTokenProvider::ClearCache()
    {
    m_tokenLifetime = 0;
    m_token = nullptr;
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
    auto finalResult = std::make_shared<SamlTokenResult>();
    return m_tokenRetrieveTask.GetTask([=]()->AsyncTaskPtr<SamlTokenResult>
        {
        LOG.infov("Requesting '%s' delegation token", m_rpUri.c_str());

        SamlTokenPtr parentToken = dynamic_pointer_cast<SamlToken>(m_parentTokenProvider->GetToken());
        if (nullptr == parentToken)
            {
            LOG.errorv("Base token not found for '%s' delegation token", m_rpUri.c_str());
            return CreateCompletedAsyncTask(SamlTokenResult::Error({}));
            }

        return m_client->RequestToken(*parentToken, m_rpUri, m_tokenRequestLifetime)
            ->Then<SamlTokenResult>([=] (SamlTokenResult result)
            {
            if (result.IsSuccess())
                LOG.infov("Received '%s' delegation token lifetime %d minutes", m_rpUri.c_str(), result.GetValue()->GetLifetime());
            return result;
            });
        })
    ->Then([=] (SamlTokenResult result)
        {
        *finalResult = result;
        if (result.IsSuccess())
            return;

        if (result.GetError().GetHttpStatus() != HttpStatus::Unauthorized)
            return;

        if (!updateBaseTokenIfFailed)
            return;

        m_parentTokenProvider->UpdateToken()->Then([=] (ISecurityTokenPtr token)
            {
            if (nullptr == token)
                return;

            RetrieveNewToken(false)->Then([=] (SamlTokenResult result)
                {
                *finalResult = result;
                });
            });
        })
    ->Then<SamlTokenResult>([=]
        {
        return *finalResult;
        });
    }
