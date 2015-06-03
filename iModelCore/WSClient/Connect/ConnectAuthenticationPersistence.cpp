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

#define SecureStoreNameSpace_ConnectLogin       "ConnectLogin"
#define SecureStoreNameSpace_ConnectToken       "ConnectToken"
#define SecureStoreKey_Token                    "Token"

#define LocalStateNameSpace_Connect             "Connect"
#define LocalStateKey_Username                  "Username"

#define LOCALSTATE_Namespace_BentleyConnect     "BentleyCONNECT"
#define LOCALSTATE_Key_Username                 "Username"
#define LOCALSTATE_Key_Token                    "Token"

bvector<std::function<void ()>> ConnectAuthenticationPersistence::s_onUserChangedCallbacks;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectAuthenticationPersistence::ConnectAuthenticationPersistence 
(
MobileDgn::ILocalState* customLocalState,
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
    if (!s_onUserChangedCallbacks.empty ())
        {
        auto oldCreds = GetCredentials ();
        if (!oldCreds.GetUsername ().empty () &&
            oldCreds.GetUsername () != credentials.GetUsername ())
            {
            for (auto& listener : s_onUserChangedCallbacks)
                {
                listener ();
                }
            }
        }

    m_localState.SaveValue (LocalStateNameSpace_Connect, LocalStateKey_Username, credentials.GetUsername ());
    m_secureStore->SaveValue (SecureStoreNameSpace_ConnectLogin, credentials.GetUsername ().c_str (), credentials.GetPassword ().c_str ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Credentials ConnectAuthenticationPersistence::GetCredentials () const
    {
    Utf8String username = m_localState.GetValue (LocalStateNameSpace_Connect, LocalStateKey_Username).asString ();
    Utf8String password = m_secureStore->LoadValue (SecureStoreNameSpace_ConnectLogin, username.c_str ());
    return Credentials (std::move (username), std::move (password));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::SetToken (SamlTokenPtr token)
    {
    m_secureStore->SaveValue (SecureStoreNameSpace_ConnectToken, SecureStoreKey_Token, token ? token->AsString ().c_str () : "");
    m_token.reset ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr ConnectAuthenticationPersistence::GetToken () const
    {
    if (nullptr == m_token)
        {
        Utf8String tokenStr = m_secureStore->LoadValue (SecureStoreNameSpace_ConnectToken, SecureStoreKey_Token);
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
    s_onUserChangedCallbacks.push_back (onUserChangedCallback);
    }
