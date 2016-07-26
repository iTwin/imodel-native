/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/IdentityTokenProvider.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "IdentityTokenProvider.h"

#include <WebServices/Connect/ImsClient.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

#define TOKEN_LIFETIME (7*24*60)
#define TOKEN_REFRESH_RATE 60

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IdentityTokenProvider::IdentityTokenProvider
(
IImsClientPtr client,
ITokenStorePtr store,
std::function<void()> tokenExpiredHandler
) :
m_client(client),
m_store(store),
m_tokenExpiredHandler(tokenExpiredHandler),
m_tokenLifetime(TOKEN_LIFETIME),
m_tokenRefreshRate(TOKEN_REFRESH_RATE)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IdentityTokenProviderPtr IdentityTokenProvider::Create(IImsClientPtr client, ITokenStorePtr store, std::function<void()> tokenExpiredHandler)
    {
    return std::shared_ptr<IdentityTokenProvider>(new IdentityTokenProvider(client, store, tokenExpiredHandler));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IdentityTokenProvider::Configure(uint32_t tokenLifetime, uint32_t tokenRefreshRate)
    {
    m_tokenLifetime = tokenLifetime;
    m_tokenRefreshRate = tokenRefreshRate;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr IdentityTokenProvider::UpdateToken()
    {
    return RenewToken()->GetResult().GetValue();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr IdentityTokenProvider::GetToken()
    {
    RenewTokenIfNeeded();
    return m_store->GetToken();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IdentityTokenProvider::RenewTokenIfNeeded()
    {
    DateTime tokenSetTime = m_store->GetTokenSetTime();
    if (!tokenSetTime.IsValid() || !ShouldRenewToken(tokenSetTime))
        return;

    auto thisPtr = shared_from_this();
    AsyncTasksManager::GetDefaultScheduler()->ExecuteAsyncWithoutAttachingToCurrentTask([=]
        {
        thisPtr->RenewToken();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SamlTokenResult> IdentityTokenProvider::RenewToken()
    {
    auto oldToken = m_store->GetToken();
    if (nullptr == oldToken)
        return CreateCompletedAsyncTask(SamlTokenResult::Error({}));

    LOG.infov("Renewing identity token");

    // TODO: avoid launching twice - use UniqueTaskHolder once its fixed
    auto thisPtr = shared_from_this();
    return m_client->RequestToken(*oldToken, nullptr, m_tokenLifetime)
        ->Then<SamlTokenResult>([=] (SamlTokenResult result)
        {
        if (result.IsSuccess())
            {
            auto newToken = result.GetValue();
            m_store->SetToken(newToken);
            LOG.infov("Renewed identity token lifetime %d minutes", newToken->GetLifetime());
            return result;
            }

        if (result.GetError().GetHttpStatus() == HttpStatus::Unauthorized)
            {
            LOG.infov("Identity token expired");
            if (m_tokenExpiredHandler)
                m_tokenExpiredHandler();
            }

        thisPtr.get();
        return result;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool IdentityTokenProvider::ShouldRenewToken(DateTimeCR tokenSetTime)
    {
    auto renewTokenAfterMs = m_tokenRefreshRate * 60 * 1000;

    int64_t unixMilliseconds;
    if (SUCCESS != tokenSetTime.ToUnixMilliseconds(unixMilliseconds))
        {
        BeAssert(false);
        return true;
        }
    unixMilliseconds += renewTokenAfterMs;
    DateTime offsetedDateTime;
    if (SUCCESS != DateTime::FromUnixMilliseconds(offsetedDateTime, unixMilliseconds))
        {
        BeAssert(false);
        return true;
        }

    auto result = DateTime::Compare(DateTime::GetCurrentTimeUtc(), offsetedDateTime);
    if (result == DateTime::CompareResult::LaterThan)
        return true;

    return false;
    }
