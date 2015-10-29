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
#include <DgnClientFx/DgnClientUi.h>
#include <WebServices/Connect/Connect.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectSpaces.h>

#include "AuthenticationData.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

#define CONNECT_SIGNED_IN                           "BentleyConnect_SignedIn"
#define BENTLEY_CONNECT_USERNAME                    "BentleyConnect_UserName"
#define BENTLEY_CONNECT_DISPLAYUSERNAME             "BentleyConnect_DisplayUserName"
#define BENTLEY_CONNECT_USERID                      "BentleyConnect_UserId"

bool GetCredentials(JsonValueCR messageDataObj, CredentialsR cred);
bool GetToken(JsonValueCR messageDataObj, Utf8StringR token);
Utf8String base64_scramble(Utf8String value, int multiplier);

void WebServices::ConnectSetup(JsonValueCR messageDataObj, bool requireToken)
    {
    Credentials cred;
    bool validCredentials = GetCredentials(messageDataObj, cred);

    Utf8String token;
    if (validCredentials)
        {
        bool tokenResult = GetToken(messageDataObj, token);

        if (!requireToken || tokenResult)
            {
            DgnClientApp::AbstractUiState().SetValue(CONNECT_SIGNED_IN, true);
            }
        else
            {
            DgnClientApp::AbstractUiState().SetValue(CONNECT_SIGNED_IN, false);
            }
        }
    else
        {
        DgnClientApp::AbstractUiState().SetValue(CONNECT_SIGNED_IN, false);
        ConnectAuthenticationPersistence::GetShared()->SetToken(nullptr);
        }

    if (!cred.GetUsername().empty())
        {
        // NOTE: Username is sent to the UI whether or not the signin was successful.
        DgnClientApp::AbstractUiState().SetValue(BENTLEY_CONNECT_USERNAME, cred.GetUsername().c_str());
        }

    if (!token.empty())
        {
        // if we have a valid token build a display name for the user from the claims in the token
        SamlToken sToken(token);

        Utf8String displayUserName = "";
        bmap<Utf8String, Utf8String> attributes;
        BentleyStatus attributeStatus = sToken.GetAttributes(attributes);
        if (SUCCESS == attributeStatus)
            {
            Utf8String givenName = attributes["givenname"];
            Utf8String surName = attributes["surname"];
            displayUserName = givenName + " " + surName;
            }
        DgnClientApp::AbstractUiState().SetValue(BENTLEY_CONNECT_DISPLAYUSERNAME, displayUserName.c_str());
        DgnClientApp::AbstractUiState().SetValue(BENTLEY_CONNECT_USERID, attributes["userid"].c_str());
        }

    Json::Value csSetup;
    csSetup[CS_MESSAGE_FIELD_username] = cred.GetUsername();
    csSetup[CS_MESSAGE_FIELD_password] = cred.GetPassword();
    csSetup[CS_MESSAGE_FIELD_token] = token;

    DgnClientUi::SendMessageToWorkThreadInternal (CS_MESSAGE_SetCredentials, std::move (csSetup));
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
