/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectAuthenticationPersistence.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNCLIENTFX

#define SecureStoreNameSpace_Connect    "Connect"
#define SecureStoreKey_Token            "Token"
#define SecureStoreKey_Username         "Username"
#define SecureStoreKey_Password         "Password"
#define SecureStoreKey_TokenSetTime     "TokenSetTime"

ConnectAuthenticationPersistencePtr ConnectAuthenticationPersistence::s_shared;
std::once_flag ConnectAuthenticationPersistence::s_shared_once;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::CustomInitialize
(
IJsonLocalState* customLocalState,
std::shared_ptr<ISecureStore> customSecureStore
)
    {
    std::call_once(s_shared_once, [] {});
    s_shared.reset(new ConnectAuthenticationPersistence(customLocalState, customSecureStore));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectAuthenticationPersistencePtr ConnectAuthenticationPersistence::GetShared()
    {
    std::call_once(s_shared_once, []
        {
        s_shared.reset(new ConnectAuthenticationPersistence());
        });

    return s_shared;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectAuthenticationPersistence::ConnectAuthenticationPersistence
(
IJsonLocalState* customLocalState,
std::shared_ptr<ISecureStore> customSecureStore
) :
m_localState(customLocalState ? *customLocalState : DgnClientFxCommon::LocalState()),
m_secureStore(customSecureStore ? customSecureStore : std::make_shared<SecureStore>(m_localState)),
m_onUserChangedKey(0)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::SetCredentials(CredentialsCR credentials)
    {
    BeMutexHolder lock (m_cs);

    UpgradeIfNeeded();

    if (!m_onUserChangedListeners.empty())
        {
        auto oldCreds = GetCredentials();
        if (!oldCreds.GetUsername().empty() &&
            oldCreds.GetUsername() != credentials.GetUsername())
            {
            for (auto& keyListener : m_onUserChangedListeners)
                {
                keyListener.second();
                }
            }
        }

    m_secureStore->SaveValue(SecureStoreNameSpace_Connect, SecureStoreKey_Username, credentials.GetUsername().c_str());
    m_secureStore->SaveValue(SecureStoreNameSpace_Connect, SecureStoreKey_Password, credentials.GetPassword().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Credentials ConnectAuthenticationPersistence::GetCredentials() const
    {
    BeMutexHolder lock (m_cs);

    UpgradeIfNeeded();

    Utf8String username = m_secureStore->LoadValue(SecureStoreNameSpace_Connect, SecureStoreKey_Username);
    Utf8String password = m_secureStore->LoadValue(SecureStoreNameSpace_Connect, SecureStoreKey_Password);

    return Credentials(std::move(username), std::move(password));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::SetToken(SamlTokenPtr token)
    {
    BeMutexHolder lock (m_cs);

    m_secureStore->SaveValue(SecureStoreNameSpace_Connect, SecureStoreKey_Token, token ? token->AsString().c_str() : "");

    Utf8String dateStr = DateTime::GetCurrentTimeUtc().ToUtf8String();
    m_secureStore->SaveValue(SecureStoreNameSpace_Connect, SecureStoreKey_TokenSetTime, token ? dateStr.c_str() : "");

    m_token.reset();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr ConnectAuthenticationPersistence::GetToken() const
    {
    BeMutexHolder lock (m_cs);

    if (nullptr == m_token)
        {
        Utf8String tokenStr = m_secureStore->LoadValue(SecureStoreNameSpace_Connect, SecureStoreKey_Token);
        if (tokenStr.empty())
            {
            return nullptr;
            }
        m_token = std::make_shared<SamlToken>(tokenStr);
        }

    return m_token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime ConnectAuthenticationPersistence::GetTokenSetTime() const
    {
    Utf8String timeStr = m_secureStore->LoadValue(SecureStoreNameSpace_Connect, SecureStoreKey_TokenSetTime);

    DateTime time;
    if (SUCCESS != DateTime::FromString(time, timeStr.c_str()))
        return DateTime();

    return time;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ConnectAuthenticationPersistence::RegisterUserChangedListener(std::function<void()> onUserChangedCallback)
    {
    BeMutexHolder lock (m_cs);

    m_onUserChangedKey++;
    m_onUserChangedListeners[m_onUserChangedKey] = onUserChangedCallback;
    return m_onUserChangedKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::UnregisterUserChangedListener(size_t key)
    {
    BeMutexHolder lock(m_cs);

    auto it = m_onUserChangedListeners.find(key);
    if (it != m_onUserChangedListeners.end())
        {
        m_onUserChangedListeners.erase(it);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::UpgradeIfNeeded() const
    {
    // Upgrade from old data that was initialized with Graphite0503 code
    // TODO: remove when Graphite0503 apps are no longer running

    // Old username storage
    Json::Value username = m_localState.GetJsonValue("Connect", "Username");
    if (username.isNull())
        {
        return;
        }

    Utf8String password = m_secureStore->LegacyLoadValue("ConnectLogin", username.asCString());
    Utf8String token = m_secureStore->LegacyLoadValue("ConnectToken", "Token");

    // Save to new storage, skip empty values to not reset existing values if any
    if (!username.empty())
        {
        m_secureStore->SaveValue(SecureStoreNameSpace_Connect, SecureStoreKey_Username, username.asCString());
        }
    if (!password.empty())
        {
        m_secureStore->SaveValue(SecureStoreNameSpace_Connect, SecureStoreKey_Password, password.c_str());
        }
    if (!token.empty())
        {
        m_secureStore->SaveValue(SecureStoreNameSpace_Connect, SecureStoreKey_Token, token.c_str());
        }

    // Remove old data
    m_secureStore->LegacyClearValue("ConnectLogin", username.asCString());
    m_secureStore->LegacyClearValue("ConnectToken", "Token");
    m_localState.SaveJsonValue("Connect", "Username", Json::nullValue);
    }
