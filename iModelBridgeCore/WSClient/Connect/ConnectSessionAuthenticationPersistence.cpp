/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectSessionAuthenticationPersistence.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "ConnectSessionAuthenticationPersistence.h"
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSessionAuthenticationPersistence::ConnectSessionAuthenticationPersistence()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSessionAuthenticationPersistence::SetCredentials(CredentialsCR credentials)
    {
    m_credentials = credentials;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Credentials ConnectSessionAuthenticationPersistence::GetCredentials() const
    {
    return m_credentials;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSessionAuthenticationPersistence::SetToken(SamlTokenPtr token)
    {
    m_token = token;
    m_tokenSetTime = token ? DateTime::GetCurrentTimeUtc() : DateTime();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr ConnectSessionAuthenticationPersistence::GetToken() const
    {
    return m_token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime ConnectSessionAuthenticationPersistence::GetTokenSetTime() const
    {
    return m_tokenSetTime;
    }
