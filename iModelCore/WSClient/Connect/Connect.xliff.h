/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/Connect.xliff.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

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
BENTLEY_TRANSLATABLE_STRINGS_END

#define ConnectLocalizedString(K) ConnectL10N::GetString(ConnectL10N::K())
