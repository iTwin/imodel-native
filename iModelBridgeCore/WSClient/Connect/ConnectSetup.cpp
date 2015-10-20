/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectSetup.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectSetup.h>

#include <Bentley/Base64Utilities.h>
#include <MobileDgn/MobileDgnUi.h>
#include <WebServices/Connect/Connect.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectSpaces.h>

#include "AuthenticationData.h"

#if defined (BENTLEY_WIN32)
#include <WebServices/Connect/IConnectTokenProvider.h>
#include <WebServices/Connect/CCConnectTokenProvider.h>
#endif

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

#define CONNECT_SIGNED_IN                           "BentleyConnect_SignedIn"
#define BENTLEY_CONNECT_USERNAME                    "BentleyConnect_UserName"
#define BENTLEY_CONNECT_DISPLAYUSERNAME             "BentleyConnect_DisplayUserName"
#define BENTLEY_CONNECT_USERID                      "BentleyConnect_UserId"
#define BENTLEY_LOGON_TOKEN_SEARCHED                "Bentley_LogonTokenSearched"

bool GetCredentials(JsonValueCR messageDataObj, CredentialsR cred);
bool GetToken(JsonValueCR messageDataObj, Utf8StringR token);
Utf8String base64_scramble(Utf8String value, int multiplier);

void WebServices::ConnectSetup(JsonValueCR messageDataObj, bool requireToken)
    {
    Credentials cred;
    bool validCredentials = GetCredentials(messageDataObj, cred);
    bool isSignedIn = false;

    Utf8String token;
    if (validCredentials)
        {
        bool tokenResult = GetToken(messageDataObj, token);

        if (!requireToken || tokenResult)
            {
            MobileDgnApplication::AbstractUiState ().SetValue (BENTLEY_LOGON_TOKEN_SEARCHED, true);
            MobileDgnApplication::AbstractUiState ().SetValue (CONNECT_SIGNED_IN, true);
            isSignedIn = true;
            }
        }
#if defined (BENTLEY_WIN32)
    else
       {
        // Do not want to get the token again when signing out, if token was already obtained
        bool searched = MobileDgnApplication::AbstractUiState ().GetValue (BENTLEY_LOGON_TOKEN_SEARCHED, false);

        if (!searched)
        {
            // Try to get the token from Bentley Logon
            std::shared_ptr<IConnectTokenProvider> tokenProvider(std::make_shared<CCConnectTokenProvider> ());
            SamlTokenPtr connectionClientToken = tokenProvider->GetToken ();
            if (connectionClientToken && !connectionClientToken->IsEmpty ())
                {
                token = connectionClientToken->AsString ();
                ConnectAuthenticationPersistence::GetShared ()->SetToken (connectionClientToken);
                MobileDgnApplication::AbstractUiState ().SetValue (CONNECT_SIGNED_IN, true);
                }
            MobileDgnApplication::AbstractUiState ().SetValue (BENTLEY_LOGON_TOKEN_SEARCHED, true);
            }
        }
#endif

    if (!cred.GetUsername().empty())
        {
        // NOTE: Username is sent to the UI whether or not the signin was successful.
        MobileDgnApplication::AbstractUiState().SetValue(BENTLEY_CONNECT_USERNAME, cred.GetUsername().c_str());
        }
    
    if (token.empty())
        {
        if (!isSignedIn)
            {
            MobileDgnApplication::AbstractUiState ().SetValue (CONNECT_SIGNED_IN, false);
            ConnectAuthenticationPersistence::GetShared ()->SetToken (nullptr);
            }
        }
    else
        {
        SamlToken sToken (token);
        Utf8String displayUserName = "";
        bmap<Utf8String, Utf8String> attributes;
        BentleyStatus attributeStatus = sToken.GetAttributes (attributes);

        if (SUCCESS == attributeStatus)
            {
            Utf8String givenName = attributes["givenname"];
            Utf8String surName = attributes["surname"];
            displayUserName = givenName + " " + surName;
            }
        MobileDgnApplication::AbstractUiState().SetValue(BENTLEY_CONNECT_DISPLAYUSERNAME, displayUserName.c_str());
        MobileDgnApplication::AbstractUiState().SetValue(BENTLEY_CONNECT_USERID, attributes["userid"].c_str());
        }

    Json::Value csSetup;
    csSetup[CS_MESSAGE_FIELD_username] = cred.GetUsername();
    csSetup[CS_MESSAGE_FIELD_password] = cred.GetPassword();
    csSetup[CS_MESSAGE_FIELD_token] = token;

    MobileDgnUi::SendMessageToWorkThread(CS_MESSAGE_SetCredentials, std::move(csSetup));
    }

bool GetCredentials(JsonValueCR messageDataObj, CredentialsR cred)
    {
    if (messageDataObj.isMember(AuthenticationData::USERNAME))
        {
        Utf8String username = messageDataObj[AuthenticationData::USERNAME].asString();
        Utf8String password = messageDataObj[AuthenticationData::PASSWORD].asString();
        cred = Credentials(username, password);
        ConnectAuthenticationPersistence::GetShared()->SetCredentials(cred);
        }
    else
        {
        cred = ConnectAuthenticationPersistence::GetShared()->GetCredentials();
        }

    return cred.IsValid();
    }

bool GetToken(JsonValueCR messageDataObj, Utf8StringR token)
    {
    if (messageDataObj.isMember(AuthenticationData::TOKEN))
        {
        token = messageDataObj[AuthenticationData::TOKEN].asString();
        ConnectAuthenticationPersistence::GetShared()->SetToken(std::make_shared<SamlToken>(token));
        }
    else
        {
        SamlTokenPtr cachedToken = ConnectAuthenticationPersistence::GetShared()->GetToken();
        if (cachedToken == nullptr || cachedToken->IsEmpty())
            {
            return false;
            }

        token = cachedToken->AsString();
        }
    return true;
    }

