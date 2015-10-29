/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/Authentication.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/Authentication.h>

#include <DgnClientFx/DgnClientUi.h>
#include <WebServices/Connect/Connect.h>
#include <WebServices/Connect/Authentication.h>

#include "AuthenticationData.h"
#include "Connect.xliff.h"

USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_WEBSERVICES

void WebServices::Authenticate(JsonValueCR messageDataObj)
    {
    Utf8String username = messageDataObj[AuthenticationData::USERNAME].asString();
    Utf8String password = messageDataObj[AuthenticationData::PASSWORD].asString();
    DGNCLIENTFX_LOGI("NavigatorApp::Msg_SignIn::Username:: %s!!", username.c_str());
    SamlToken token;
    StatusInt result = Connect::BC_ERROR;
    bool isSignOut = false;

    if (password.empty())
        {
        isSignOut = true;
        }
    else
        {
        result = Connect::Login(Credentials(username, password), token);
        }

    if (Connect::BC_SUCCESS != result)
        {
        Json::Value setupData(Json::objectValue);
        setupData[AuthenticationData::USERNAME] = username;

        // XXX: duplicate properties under Data. Quick workaround for WorkSite
        // demo, to be unified after.
        Json::Value data(Json::objectValue);
        data[AuthenticationData::USERNAME] = username;
        setupData["Data"] = data;

        DgnClientUi::SendMessageToWorkThreadInternal(CONNECT_REQUEST_SETUP, std::move (setupData));

        if (isSignOut)
            {
            DgnClientApp::App().Messages().Send(NotificationMessage("FieldApps.Message.Connect.SignOut_Succeeded"));
            }
        else
            {
            Utf8String message;
            if (result == Connect::BC_ERROR)
                {
                // Server error
                message = ConnectLocalizedString(ALERT_SignInFailed_ServerError);
                }
            else
                {
                //
                message = ConnectLocalizedString(ALERT_SignInFailed_Message);
                }
            Json::Value messageValue(Json::objectValue);
            messageValue["message"] = message;
            DgnClientApp::App().Messages().Send(JsonMessage("FieldApps.Message.Connect.SignIn_Failed", messageValue));
            }
        return;
        }

    DgnClientApp::App().Messages().Send(NotificationMessage("FieldApps.Message.Connect.SignIn_Succeeded"));
    Json::Value userData(Json::objectValue);
    userData[AuthenticationData::USERNAME] = username;
    userData[AuthenticationData::PASSWORD] = password;
    userData[AuthenticationData::TOKEN] = token.AsString();
    DgnClientApp::App().Messages().Send(JsonMessage(CONNECT_COMMAND_SHOW_USER_DATA, userData));

    Json::Value setupData(messageDataObj);
    setupData[AuthenticationData::TOKEN] = token.AsString();

    // XXX: duplicate properties under Data. Quick workaround for WorkSite
    // demo, to be unified after.
    Json::Value data(messageDataObj);
    data[AuthenticationData::TOKEN] = token.AsString();
    setupData["Data"] = data;

    DgnClientUi::SendMessageToWorkThreadInternal(CONNECT_REQUEST_SETUP, std::move(setupData));
    }
