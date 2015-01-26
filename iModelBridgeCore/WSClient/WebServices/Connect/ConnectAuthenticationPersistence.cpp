/*--------------------------------------------------------------------------------------+
|
|     $Source: WebServices/Connect/ConnectAuthenticationPersistence.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WebServicesInternal.h"
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/SecureStore.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN

#define SecureStoreNameSpace_ConnectLogin       "ConnectLogin"
#define SecureStoreNameSpace_ConnectToken       "ConnectToken"
#define SecureStoreKey_Token                    "Token"

#define LocalStateNameSpace_Connect             "Connect"
#define LocalStateKey_Username                  "Username"
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

#define LOCALSTATE_Namespace_BentleyConnect "BentleyCONNECT"
#define LOCALSTATE_Key_Username             "Username"
#define LOCALSTATE_Key_Token                "Token"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectAuthenticationPersistence::ConnectAuthenticationPersistence (MobileDgn::ILocalState* customLocalState) :
m_localState (customLocalState ? *customLocalState : MobileDgnApplication::App ().LocalState ())
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::SetCredentials (CredentialsCR credentials)
    {
    m_localState.SaveValue (LocalStateNameSpace_Connect, LocalStateKey_Username, credentials.GetUsername ());
    SecureStore (&m_localState).SaveValue (SecureStoreNameSpace_ConnectLogin, credentials.GetUsername ().c_str (), credentials.GetPassword ().c_str ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Credentials ConnectAuthenticationPersistence::GetCredentials () const
    {
    Utf8String username = m_localState.GetValue (LocalStateNameSpace_Connect, LocalStateKey_Username).asString ();
    Utf8String password = SecureStore (&m_localState).LoadValue (SecureStoreNameSpace_ConnectLogin, username.c_str ());
    return Credentials (std::move (username), std::move (password));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::SetToken (SamlTokenPtr token)
    {
    SecureStore (&m_localState).SaveValue (SecureStoreNameSpace_ConnectToken, SecureStoreKey_Token, token ? token->AsString ().c_str () : "");
    m_token.reset ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr ConnectAuthenticationPersistence::GetToken () const
    {
    if (nullptr == m_token)
        {
        Utf8String tokenStr = SecureStore (&m_localState).LoadValue (SecureStoreNameSpace_ConnectToken, SecureStoreKey_Token);
        if (tokenStr.empty ())
            {
            return nullptr;
            }
        m_token = std::make_shared<SamlToken> (tokenStr);
        }

    return m_token;
    }
