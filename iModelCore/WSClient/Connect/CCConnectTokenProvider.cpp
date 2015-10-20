/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/CCConnectTokenProvider.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/CCConnectTokenProvider.h>

#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/Connect.h>
#include <Bentley/Base64Utilities.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectClientCCApiWrapper.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

#define BENTLEY_LOGON_TOKEN_SEARCHED                "Bentley_LogonTokenSearched"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Jahan.Zeb    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CCConnectTokenProvider::CCConnectTokenProvider (std::shared_ptr<IConnectAuthenticationPersistence> customPersistence) :
m_persistence (customPersistence ? customPersistence : ConnectAuthenticationPersistence::GetShared ())
    {

    // Convert baseServiceAddress to wchar_t* to be used as input parameter in GetDelegateSecurityToken
    //size_t sizeBuff = CacheManagerUtils::Utf8StringToWchar_t (relyingPartyAddress, NULL, 0);
    //m_relyingPartyAddress = new wchar_t[sizeBuff];
    //CacheManagerUtils::Utf8StringToWchar_t (relyingPartyAddress, m_relyingPartyAddress, sizeBuff);

    m_token = GetTokenFromConnectClient ();
    m_persistence->SetToken (m_token);

    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Brad.Hadden    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr CCConnectTokenProvider::UpdateToken ()
    {
    bool tryConnect = MobileDgnApplication::AbstractUiState ().GetValue (BENTLEY_LOGON_TOKEN_SEARCHED, false);

    if (tryConnect)
        {
        // Try to get token from CONNECT Client first
        m_token = GetTokenFromConnectClient ();
        if (m_token && m_token->AsString ().length () != 0)
            {
            m_persistence->SetToken (m_token);
            return m_token;
            }
        }

    Credentials creds = m_persistence->GetCredentials ();

    auto token = std::make_shared<SamlToken> ();
    if (!creds.IsValid () || 0 != Connect::Login (creds, *token))
        {
        return nullptr;
        }

    m_persistence->SetToken (token);
    return token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Brad.Hadden    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr CCConnectTokenProvider::GetToken ()
    {
    return m_persistence->GetToken ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Brad.Hadden    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr CCConnectTokenProvider::GetTokenFromConnectClient ()
    {
    SamlTokenPtr samlToken = nullptr;

    try
        {
        Utf8String stsUrl = UrlProvider::Urls::ImsStsAuth.Get ();
        std::string sts = stsUrl.c_str ();
        std::wstring stsRelyingParty;
        stsRelyingParty.assign (sts.begin (), sts.end ());

        ConnectClientCCApiWrapper ccapi;

        Utf8String stsToken (ccapi.GetSerializedDelegateSecurityToken (stsRelyingParty).c_str ());
        // Get SamlToken 
        samlToken = std::make_shared<SamlToken> (Base64Utilities::Decode (stsToken));
        
        }
    catch (std::exception exp)
        {
        samlToken = nullptr;
        }

    return samlToken;
    }
