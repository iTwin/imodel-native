/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/Connect.xliff.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    };
MOBILEDGN_TRANSLATABLE_STRINGS_END

#define ConnectLocalizedString(K) ConnectL10N::GetString(ConnectL10N::K, #K)
