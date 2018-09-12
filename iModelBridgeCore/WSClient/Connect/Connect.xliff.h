/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/Connect.xliff.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <MobileDgn/MobileDgnL10N.h>

//=======================================================================================
// @bsiclass
//=======================================================================================
MOBILEDGN_TRANSLATABLE_STRINGS_START(ConnectL10N, Connect)
    {
    ALERT_UnsupportedToken,                  // =="Could not sign in with provided token."==
    ALERT_SignInFailed_ServerError,          // =="Could not connect to server. Check your network connection."==
    ALERT_SignInFailed_Message,              // =="Please enter the correct email and password."==
    ALERT_ConnectionClientNotLoggedIn_Message,             // =="Please sign in to CONNECTION Client with your CONNECT organization user account."==
    ALERT_CCNotInstalledError,               // =="CONNECTION Client is not installed. Please install it and try again."==
    ALERT_CCNotRunningError,                 // =="CONNECTION Client is not running. Please run it and try again."==
    ALERT_CCInvalidCredentialsError,         // =="Invalid username or password in CONNECTION Client. Please try signing in again."==
    ALERT_CCUnhandledExceptionError,         // =="Unknown error in CONNECTION Client. Please contact your administrator."==
    ALERT_CCNotAcceptedEulaError,            // =="EULA not accepted. Please try signing in again."==
    ALERT_CCUnableToStartAppError,           // =="Unable to start CONNECTION Client. Please contact your administrator."==
    ALERT_CCApiObsoleteError,                // =="Incompatible CONNECTION Client version. Install compatible CONNECTION Client version or contact your administrator."==
    ALERT_CCServiceUnavailableError,         // =="Service unavailable. Please check your internet connection."==
    ALERT_CCUserNotAffiliatedError,          // =="The account you use in the CONNECTION Client must be affiliated with your CONNECT organization. Please contact your administrator."==
    ALERT_CCUnknownError,                    // =="Unknown error communicating with CONNECTION Client. Please contact your administrator."==
    };
MOBILEDGN_TRANSLATABLE_STRINGS_END

#define ConnectLocalizedString(K) ConnectL10N::GetString(ConnectL10N::K, #K)
