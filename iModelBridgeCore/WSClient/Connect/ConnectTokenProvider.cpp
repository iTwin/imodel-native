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
#include <WebServices/Connect/Connect.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

#define TOKEN_LIFETIME (7*24*60)
#define TOKEN_REFRESH_RATE 60

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectTokenProvider::ConnectTokenProvider
(
IConnectAuthenticationPersistencePtr customPersistence,
bool isTokenBasedAuthentication,
std::function<void()> tokenExpiredHandler
) :
m_persistence(customPersistence ? customPersistence : ConnectAuthenticationPersistence::GetShared()),
m_isTokenBasedAuthentication(isTokenBasedAuthentication),
m_tokenExpiredHandler(tokenExpiredHandler),
m_tokenLifetime(TOKEN_LIFETIME),
m_tokenRefreshRate(TOKEN_REFRESH_RATE)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectTokenProvider::Configure(uint64_t tokenLifetime, uint64_t tokenRefreshRate)
    {
    m_tokenLifetime = tokenLifetime;
    m_tokenRefreshRate = tokenRefreshRate;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr ConnectTokenProvider::UpdateToken()
    {
    if (m_isTokenBasedAuthentication)
        {
        if (m_persistence->GetToken() != nullptr)
            {
            if (m_tokenExpiredHandler)
                m_tokenExpiredHandler();
            }
        return nullptr;
        }

    Credentials creds = m_persistence->GetCredentials();
    if (!creds.IsValid())
        return nullptr;

    auto result = Connect::Login(creds, nullptr, nullptr, m_tokenLifetime)->GetResult();
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
    if (m_isTokenBasedAuthentication)
        {
        // TODO: Launch token renewal asynchronously and just return current token
        // Check if token was issued more than 1 hour ago. If so, renew it.
        DateTime tokenSetTime = m_persistence->GetTokenSetTime();
        if (tokenSetTime.IsValid() && ShouldRenewToken(tokenSetTime))
            {
            auto oldToken = m_persistence->GetToken();
            if (nullptr == oldToken)
                return nullptr;

            auto tokenResult = Connect::RenewToken(*oldToken, nullptr, nullptr, m_tokenLifetime)->GetResult();
            if (!tokenResult.IsSuccess())
                return nullptr;

            m_persistence->SetToken(tokenResult.GetValue());
            }
        }
    return m_persistence->GetToken();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectTokenProvider::ShouldRenewToken(DateTimeCR tokenSetTime)
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
