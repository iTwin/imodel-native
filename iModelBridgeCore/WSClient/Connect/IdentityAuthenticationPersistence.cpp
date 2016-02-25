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
USING_NAMESPACE_BENTLEY_MOBILEDGN
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

#define SecureStoreNameSpace_Connect    "Connect"
#define SecureStoreKey_Token            "IdentityToken"
#define SecureStoreKey_TokenSetTime     "IdentityTokenSetTime"

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IdentityAuthenticationPersistence::IdentityAuthenticationPersistence(std::shared_ptr<ISecureStore> customSecureStore) :
m_secureStore(customSecureStore ? customSecureStore : std::make_shared<SecureStore>())
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
    BeCriticalSectionHolder lock(m_cs);

    m_secureStore->SaveValue(SecureStoreNameSpace_Connect, SecureStoreKey_Token, token ? token->AsString().c_str() : "");

    Utf8String dateStr = DateTime::GetCurrentTimeUtc().ToUtf8String();
    m_secureStore->SaveValue(SecureStoreNameSpace_Connect, SecureStoreKey_TokenSetTime, token ? dateStr.c_str() : "");

    m_token.reset();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr IdentityAuthenticationPersistence::GetToken() const
    {
    BeCriticalSectionHolder lock(m_cs);

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
DateTime IdentityAuthenticationPersistence::GetTokenSetTime() const
    {
    Utf8String timeStr = m_secureStore->LoadValue(SecureStoreNameSpace_Connect, SecureStoreKey_TokenSetTime);

    DateTime time;
    if (SUCCESS != DateTime::FromString(time, timeStr.c_str()))
        return DateTime();

    return time;
    }
