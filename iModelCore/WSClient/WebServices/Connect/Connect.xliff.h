/*--------------------------------------------------------------------------------------+
|
|     $Source: WebServices/Connect/Connect.xliff.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <MobileDgn/MobileDgnL10N.h>

//=======================================================================================
// @bsiclass                                      
//=======================================================================================
MOBILEDGN_TRANSLATABLE_STRINGS_START (ConnectL10N, Connect)
    {
    ALERT_SignInFailed_ServerError,          // =="Could not connect to server. Check your network connection."==
    ALERT_SignInFailed_Message,              // =="Please enter the correct email and password."==
    FIELDAPPS_SingleSignOn,                  // =="Single sign-on"==
    FIELDAPPS_DontHaveAnAccount,             // =="Don't have an account yet?"==
    FIELDAPPS_SignUp,                        // =="Sign up"==
    FIELDAPPS_Email,                         // =="Email"==
    FIELDAPPS_Password,                      // =="Password"==
    FIELDAPPS_ForgotPassword,                // =="Forgot Password?"==
    FIELDAPPS_SignIn,                        // =="Sign in"==
    FIELDAPPS_ProductDemo,                   // =="Product Demo"==
    FIELDAPPS_EmailAndPasswordRequired,      // =="You must enter an email address and password."==
    FIELDAPPS_Error,                         // =="Error"==
    FIELDAPPS_SignInFailed,                  // =="Sign In Failed"==
    FIELDAPPS_UnknownError,                  // =="Unknown error."==
    FIELDAPPS_SignInUsername,                // =="Signed in username:"==
    FIELDAPPS_SignOut,                       // =="Sign Out"==
    FIELDAPPS_SigningInDotDotDot,            // =="Signing In"==
    };
MOBILEDGN_TRANSLATABLE_STRINGS_END

#define ConnectLocalizedString(K) ConnectL10N::GetString(ConnectL10N::K, #K)
