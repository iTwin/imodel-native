/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/IdentityAuthenticationPersistence.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "IdentityAuthenticationPersistence.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNCLIENTFX

#define LocalState_NameSpace        "Connect"
#define LocalState_Token            "IdentityToken"
#define LocalState_TokenSetTime     "IdentityTokenSetTime"

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IdentityAuthenticationPersistence::IdentityAuthenticationPersistence
(
IJsonLocalState* localState,
std::shared_ptr<ISecureStore> customSecureStore
) :
m_localState(localState ? *localState : DgnClientFxCommon::LocalState()),
m_secureStore(customSecureStore ? customSecureStore : std::make_shared<SecureStore>(m_localState))
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IdentityAuthenticationPersistence::SetCredentials(CredentialsCR credentials)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Credentials IdentityAuthenticationPersistence::GetCredentials() const
    {
    return Credentials();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IdentityAuthenticationPersistence::SetToken(SamlTokenPtr token)
    {
    BeMutexHolder lock(m_cs);

    Utf8String tokenStr = token ? m_secureStore->Encrypt(token->AsString().c_str()) : "";
    m_localState.SaveJsonValue(LocalState_NameSpace, LocalState_Token, tokenStr);

    Utf8String dateStr = DateTime::GetCurrentTimeUtc().ToUtf8String();
    m_localState.SaveJsonValue(LocalState_NameSpace, LocalState_TokenSetTime, token ? dateStr.c_str() : "");

    m_token.reset();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr IdentityAuthenticationPersistence::GetToken() const
    {
    BeMutexHolder lock(m_cs);

    if (nullptr == m_token)
        {
        Utf8String tokenStr = m_localState.GetJsonValue(LocalState_NameSpace, LocalState_Token).asString();
        if (tokenStr.empty())
            return nullptr;

        m_token = std::make_shared<SamlToken>(m_secureStore->Decrypt(tokenStr.c_str()));
        }

    return m_token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime IdentityAuthenticationPersistence::GetTokenSetTime() const
    {
    Utf8String timeStr = m_localState.GetJsonValue(LocalState_NameSpace, LocalState_TokenSetTime).asString();

    DateTime time;
    if (SUCCESS != DateTime::FromString(time, timeStr.c_str()))
        return DateTime();

    return time;
    }
