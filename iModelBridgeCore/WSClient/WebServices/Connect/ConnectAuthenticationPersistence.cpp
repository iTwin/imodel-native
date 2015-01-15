/*--------------------------------------------------------------------------------------+
|
|     $Source: WebServices/Connect/ConnectAuthenticationPersistence.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WebServicesInternal.h"
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/PasswordPersistence.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN
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
    m_localState.SaveValue (LOCALSTATE_Namespace_BentleyConnect, LOCALSTATE_Key_Username, credentials.GetUsername ());
    PasswordPersistence persistence (&m_localState);
    persistence.SavePassword (credentials.GetUsername ().c_str (), credentials.GetPassword ().c_str ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Credentials ConnectAuthenticationPersistence::GetCredentials () const
    {
    Utf8String username = m_localState.GetValue (LOCALSTATE_Namespace_BentleyConnect, LOCALSTATE_Key_Username).asString ();
    PasswordPersistence persistence (&m_localState);
    Utf8String password = persistence.LoadPassword (username.c_str ());
    return Credentials (std::move (username), std::move (password));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationPersistence::SetToken (SamlTokenPtr token)
    {
    m_localState.SaveValue (LOCALSTATE_Namespace_BentleyConnect, LOCALSTATE_Key_Token, token ? token->AsString () : nullptr);
    m_token = token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr ConnectAuthenticationPersistence::GetToken () const
    {
    if (nullptr == m_token)
        {
        Utf8String tokenStr = m_localState.GetValue (LOCALSTATE_Namespace_BentleyConnect, LOCALSTATE_Key_Token).asString ();
        if (tokenStr.empty ())
            {
            return nullptr;
            }
        m_token = std::make_shared<SamlToken> (tokenStr);
        }

    return m_token;
    }
