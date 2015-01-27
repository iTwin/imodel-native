/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/Connect/ConnectSetup.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ClientInternal.h"
#include <WebServices/Client/Connect/ConnectSetup.h>
#include "AuthenticationData.h"
#include <MobileDgn/MobileDgnUi.h>
#include <WebServices/Client/Connect/ConnectAuthenticationPersistence.h>
#include "DatabaseHelper.h"
#include <Bentley/Base64Utilities.h>
#include <WebServices/Client/Connect/Connect.h>
#include <WebServices/Client/Connect/ConnectSpaces.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

#define CONNECT_SIGNED_IN                           "BentleyConnect_SignedIn"
#define BENTLEY_CONNECT_USERNAME                    "BentleyConnect_UserName"

bool GetCredentials (JsonValueCR messageDataObj, CredentialsR cred);
bool GetToken (JsonValueCR messageDataObj, Utf8StringR token);
Utf8String base64_scramble (Utf8String value, int multiplier);

void ConnectSetup (JsonValueCR messageDataObj, bool requireToken)
    {
    Credentials cred;
    bool validCredentials = GetCredentials (messageDataObj, cred);

    Utf8String token;
    if (validCredentials)
        {
        bool tokenResult = GetToken (messageDataObj, token);

        if (!requireToken || tokenResult)
            {
            MobileDgnApplication::AbstractUiState ().SetValue (CONNECT_SIGNED_IN, true);
            }
        else
            {
            MobileDgnApplication::AbstractUiState ().SetValue (CONNECT_SIGNED_IN, false);
            }
        }
    else
        {
        MobileDgnApplication::AbstractUiState ().SetValue (CONNECT_SIGNED_IN, false);
        ConnectAuthenticationPersistence caPersistence;
        caPersistence.SetToken (nullptr);
        }

    if (!cred.GetUsername ().empty ())
        {
        // NOTE: Username is sent to the UI whether or not the signin was successful.
        MobileDgnApplication::AbstractUiState ().SetValue (BENTLEY_CONNECT_USERNAME, cred.GetUsername ().c_str ());
        }
    Json::Value csSetup;
    csSetup[CS_MESSAGE_FIELD_username] = cred.GetUsername ();
    csSetup[CS_MESSAGE_FIELD_password] = cred.GetPassword ();
    csSetup[CS_MESSAGE_FIELD_token] = token;

    MobileDgnUi::SendMessageToWorkThread (CS_MESSAGE_SetCredentials, csSetup);
    }

bool GetCredentials (JsonValueCR messageDataObj, CredentialsR cred)
    {
    ConnectAuthenticationPersistence caPersistence;

    if (messageDataObj.isMember (AuthenticationData::USERNAME))
        {
        Utf8String username = messageDataObj[AuthenticationData::USERNAME].asString ();
        Utf8String password = messageDataObj[AuthenticationData::PASSWORD].asString ();
        cred = Credentials (username, password);
        caPersistence.SetCredentials (cred);
        }
    else
        {
        cred = caPersistence.GetCredentials ();
        }

    return cred.IsValid ();
    }

bool GetToken (JsonValueCR messageDataObj, Utf8StringR token)
    {
    ConnectAuthenticationPersistence caPersistence;

    if (messageDataObj.isMember (AuthenticationData::TOKEN))
        {
        token = messageDataObj[AuthenticationData::TOKEN].asString ();
        caPersistence.SetToken (std::make_shared<SamlToken> (token));
        }
    else
        {
        SamlTokenPtr cachedToken = caPersistence.GetToken ();
        if (cachedToken == nullptr || cachedToken->IsEmpty ())
            {
            return false;
            }

        token = cachedToken->AsString ();
        }
    return true;
    }
