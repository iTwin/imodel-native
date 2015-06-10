/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectAuthenticationPersistence.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

#define SecureStoreNameSpace_Connect    "Connect"
#define SecureStoreKey_Token            "Token"
#define SecureStoreKey_Username         "Username"
#define SecureStoreKey_Password         "Password"

ConnectAuthenticationPersistencePtr ConnectAuthenticationPersistence::s_shared;
std::once_flag ConnectAuthenticationPersistence::s_shared_once;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::CustomInitialize
(
ILocalState* customLocalState,
std::shared_ptr<ISecureStore> customSecureStore
)
    {
    std::call_once (s_shared_once, [] {});
    s_shared.reset (new ConnectAuthenticationPersistence (customLocalState, customSecureStore));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectAuthenticationPersistencePtr ConnectAuthenticationPersistence::GetShared ()
    {
    std::call_once (s_shared_once, []
        {
        s_shared.reset (new ConnectAuthenticationPersistence ());
        });

    return s_shared;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectAuthenticationPersistence::ConnectAuthenticationPersistence
(
ILocalState* customLocalState,
std::shared_ptr<ISecureStore> customSecureStore
) :
m_localState (customLocalState ? *customLocalState : MobileDgnApplication::App ().LocalState ()),
m_secureStore (customSecureStore ? customSecureStore : std::make_shared<SecureStore> (&m_localState))
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::SetCredentials (CredentialsCR credentials)
    {
    BeCriticalSectionHolder lock (m_cs);

    UpgradeIfNeeded ();

    if (!m_onUserChangedCallbacks.empty ())
        {
        auto oldCreds = GetCredentials ();
        if (!oldCreds.GetUsername ().empty () &&
            oldCreds.GetUsername () != credentials.GetUsername ())
            {
            for (auto& listener : m_onUserChangedCallbacks)
                {
                listener ();
                }
            }
        }

    m_secureStore->SaveValue (SecureStoreNameSpace_Connect, SecureStoreKey_Username, credentials.GetUsername ().c_str ());
    m_secureStore->SaveValue (SecureStoreNameSpace_Connect, SecureStoreKey_Password, credentials.GetPassword ().c_str ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Credentials ConnectAuthenticationPersistence::GetCredentials () const
    {
    BeCriticalSectionHolder lock (m_cs);

    UpgradeIfNeeded ();

    Utf8String username = m_secureStore->LoadValue (SecureStoreNameSpace_Connect, SecureStoreKey_Username);
    Utf8String password = m_secureStore->LoadValue (SecureStoreNameSpace_Connect, SecureStoreKey_Password);

    return Credentials (std::move (username), std::move (password));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::SetToken (SamlTokenPtr token)
    {
    BeCriticalSectionHolder lock (m_cs);
    m_secureStore->SaveValue (SecureStoreNameSpace_Connect, SecureStoreKey_Token, token ? token->AsString ().c_str () : "");
    m_token.reset ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr ConnectAuthenticationPersistence::GetToken () const
    {
    BeCriticalSectionHolder lock (m_cs);

    if (nullptr == m_token)
        {
        Utf8String tokenStr = m_secureStore->LoadValue (SecureStoreNameSpace_Connect, SecureStoreKey_Token);
        if (tokenStr.empty ())
            {
            return nullptr;
            }
        m_token = std::make_shared<SamlToken> (tokenStr);
        }

    return m_token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::RegisterUserChangedListener (std::function<void ()> onUserChangedCallback)
    {
    BeCriticalSectionHolder lock (m_cs);
    m_onUserChangedCallbacks.push_back (onUserChangedCallback);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::UpgradeIfNeeded () const
    {
    // Upgrade from old data that was initialized with Graphite0503 code
    // TODO: remove when Graphite0503 apps are no longer running

    // Old username storage
    Json::Value username = m_localState.GetValue ("Connect", "Username");
    if (username.isNull ())
        {
        return;
        }

    Utf8String password = m_secureStore->LegacyLoadValue ("ConnectLogin", username.asCString ());
    Utf8String token = m_secureStore->LegacyLoadValue ("ConnectToken", "Token");

    // Save to new storage, skip empty values to not reset existing values if any
    if (!username.empty ())
        {
        m_secureStore->SaveValue (SecureStoreNameSpace_Connect, SecureStoreKey_Username, username.asCString ());
        }
    if (!password.empty ())
        {
        m_secureStore->SaveValue (SecureStoreNameSpace_Connect, SecureStoreKey_Password, password.c_str ());
        }
    if (!token.empty ())
        {
        m_secureStore->SaveValue (SecureStoreNameSpace_Connect, SecureStoreKey_Token, token.c_str ());
        }

    // Remove old data
    m_secureStore->LegacyClearValue ("ConnectLogin", username.asCString ());
    m_secureStore->LegacyClearValue ("ConnectToken", "Token");
    m_localState.SaveValue ("Connect", "Username", Json::nullValue);
    }
