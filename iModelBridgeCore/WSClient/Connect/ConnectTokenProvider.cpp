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

#define RENEW_TOKEN_AFTER_MS (60*60*1000)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectTokenProvider::ConnectTokenProvider
(
std::shared_ptr<IConnectAuthenticationPersistence> customPersistence,
bool isTokenBasedAuthentication,
std::function<void()> tokenExpiredHandler
) :
m_persistence(customPersistence ? customPersistence : ConnectAuthenticationPersistence::GetShared()),
m_isTokenBasedAuthentication(isTokenBasedAuthentication),
m_tokenExpiredHandler(tokenExpiredHandler)
    {}

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
    if (m_isTokenBasedAuthentication)
        {
        // TODO: Launch token renewal asynchronously and just return current token
        // Check if token was issued more than 1 hour ago. If so, renew it.
        DateTime tokenSetTime = m_persistence->GetTokenSetTime();
        if (tokenSetTime.IsValid() && ShouldRenewToken(tokenSetTime, RENEW_TOKEN_AFTER_MS))
            {
            auto oldToken = m_persistence->GetToken();
            auto newToken = std::make_shared<SamlToken>();
            if (nullptr == oldToken || SUCCESS != Connect::RenewToken(*oldToken, *newToken))
                {
                return nullptr;
                }

            m_persistence->SetToken(newToken);
            }
        }
    return m_persistence->GetToken();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectTokenProvider::ShouldRenewToken(DateTime tokenSetTime, int64_t renewTokenAfter)
    {
    int64_t unixMilliseconds;
    if (SUCCESS != tokenSetTime.ToUnixMilliseconds(unixMilliseconds))
        {
        BeAssert(false);
        return true;
        }
    unixMilliseconds += renewTokenAfter;
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
