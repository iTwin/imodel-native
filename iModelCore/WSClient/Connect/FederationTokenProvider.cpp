/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/FederationTokenProvider.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/FederationTokenProvider.h>

#include <WebServices/Connect/FederationAuthenticationPersistence.h>
#include <WebServices/Connect/Connect.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

#define TOKEN_LIFETIME (7*24*60)
#define TOKEN_REFRESH_RATE 60

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
FederationTokenProvider::FederationTokenProvider
(
ITokenStorePtr store,
std::function<void()> tokenExpiredHandler
) :
m_store(store),
m_tokenExpiredHandler(tokenExpiredHandler),
m_tokenLifetime(TOKEN_LIFETIME),
m_tokenRefreshRate(TOKEN_REFRESH_RATE)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FederationTokenProvider::Configure(uint64_t tokenLifetime, uint64_t tokenRefreshRate)
    {
    m_tokenLifetime = tokenLifetime;
    m_tokenRefreshRate = tokenRefreshRate;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr FederationTokenProvider::UpdateToken()
    {
    if (m_store->GetToken() != nullptr)
        {
        if (m_tokenExpiredHandler)
            m_tokenExpiredHandler();
        }
    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr FederationTokenProvider::GetToken()
    {
    DateTime tokenSetTime = m_store->GetTokenSetTime();
    if (tokenSetTime.IsValid() && ShouldRenewToken(tokenSetTime))
        {
        auto oldToken = m_store->GetToken();
        if (nullptr == oldToken)
            return nullptr;

        AsyncTasksManager::GetDefaultScheduler()->ExecuteAsyncWithoutAttachingToCurrentTask([=]
            {
            // TODO: avoid launching twice - use UniqueTaskHolder once its fixed
            LOG.infov("Renewing identity token");
            Connect::RenewToken(*oldToken, nullptr, nullptr, m_tokenLifetime)
                ->Then([=] (SamlTokenResult result)
                {
                if (!result.IsSuccess())
                    return;

                auto token = result.GetValue();
                m_store->SetToken(token);
                LOG.infov("Renewed identity token lifetime %d minutes", token->GetLifetime());
                });
            });
        }

    return m_store->GetToken();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool FederationTokenProvider::ShouldRenewToken(DateTimeCR tokenSetTime)
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
