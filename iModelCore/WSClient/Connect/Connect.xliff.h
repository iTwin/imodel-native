/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <BeSQLite/L10N.h>

//=======================================================================================
// @bsiclass
//=======================================================================================
BENTLEY_TRANSLATABLE_STRINGS_START (ConnectL10N, Connect)
    L10N_STRING(ALERT_UnsupportedToken)                     // =="Could not sign in with provided token."==
    L10N_STRING(ALERT_SignInFailed_ServerError)             // =="Could not connect to server. Check your network connection."==
    L10N_STRING(ALERT_SignInFailed_Message)                 // =="Please enter the correct email and password."==
    L10N_STRING(ALERT_ConnectionClientNotLoggedIn_Message)  // =="Please log in to CONNECTION Client."==
    L10N_STRING(ALERT_CCNotInstalledError)                  // =="CONNECTION Client is not installed. Please install it and try again."==
    L10N_STRING(ALERT_CCNotRunningError)                    // =="CONNECTION Client is not running. Please run it and try again."==
    L10N_STRING(ALERT_CCInvalidCredentialsError)            // =="Invalid username or password in CONNECTION Client. Please try signing in again."==
    L10N_STRING(ALERT_CCUnhandledExceptionError)            // =="Unknown error in CONNECTION Client. Please contact your administrator."==
    L10N_STRING(ALERT_CCNotAcceptedEulaError)               // =="EULA not accepted. Please try signing in again."==
    L10N_STRING(ALERT_CCUnableToStartAppError)              // =="Unable to start CONNECTION Client. Please contact your administrator."==
    L10N_STRING(ALERT_CCApiObsoleteError)                   // =="Incompatible CONNECTION Client version. Install compatible CONNECTION Client version or contact your administrator."==
    L10N_STRING(ALERT_CCServiceUnavailableError)            // =="Service unavailable. Please check your internet connection."==
    L10N_STRING(ALERT_CCUserNotAffiliatedError)             // =="The account you use in the CONNECTION Client must be affiliated with your CONNECT organization. Please contact your administrator."==
    L10N_STRING(ALERT_CCUnknownError)                       // =="Unknown error communicating with CONNECTION Client. Please contact your administrator."==
BENTLEY_TRANSLATABLE_STRINGS_END

#define ConnectLocalizedString(K) ConnectL10N::GetString(ConnectL10N::K())
