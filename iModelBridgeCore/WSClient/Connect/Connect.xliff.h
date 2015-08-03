/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/Connect.xliff.h $
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
    L10N_STRING(ALERT_SignInFailed_ServerError)          // =="Could not connect to server. Check your network connection."==
    L10N_STRING(ALERT_SignInFailed_Message)              // =="Please enter the correct email and password."==
    L10N_STRING(FIELDAPPS_SingleSignOn)                  // =="Single sign-on"==
    L10N_STRING(FIELDAPPS_DontHaveAnAccount)             // =="Don't have an account yet?"==
    L10N_STRING(FIELDAPPS_SignUp)                        // =="Sign up"==
    L10N_STRING(FIELDAPPS_SignUpSuggestion)              // =="Don't have a profile?"==
    L10N_STRING(FIELDAPPS_SignUpSuggestionLink)          // =="Register"==
    L10N_STRING(FIELDAPPS_Email)                         // =="Email"==
    L10N_STRING(FIELDAPPS_Password)                      // =="Password"==
    L10N_STRING(FIELDAPPS_ForgotPassword)                // =="Forgot Password?"==
    L10N_STRING(FIELDAPPS_SignIn)                        // =="Sign in"==
    L10N_STRING(FIELDAPPS_ProductDemo)                   // =="Demo"==
    L10N_STRING(FIELDAPPS_EmailAndPasswordRequired)      // =="You must enter an email address and password."==
    L10N_STRING(FIELDAPPS_Error)                         // =="Error"==
    L10N_STRING(FIELDAPPS_SignInFailed)                  // =="Sign In Failed"==
    L10N_STRING(FIELDAPPS_UnknownError)                  // =="Unknown error."==
    L10N_STRING(FIELDAPPS_SignInUsername)                // =="Signed in username:"==
    L10N_STRING(FIELDAPPS_SignOut)                       // =="Sign Out"==
    L10N_STRING(FIELDAPPS_SigningInDotDotDot)            // =="Signing In..."==
    L10N_STRING(FIELDAPPS_EulaAccept)                    // =="Accept"==
    L10N_STRING(FIELDAPPS_EulaDecline)                   // =="Decline"==
    L10N_STRING(FIELDAPPS_EulaAgree)                     // =="To continue you must accept the terms in the Licence Agreement"==
    L10N_STRING(FIELDAPPS_EulaHeader)                    // =="End-user license agreement"==
    L10N_STRING(FIELDAPPS_EulaChecking)                  // =="Checking EULA..."==
    L10N_STRING(FIELDAPPS_EulaAccepting)                 // =="Accepting EULA..."==
MOBILEDGN_TRANSLATABLE_STRINGS_END

#define ConnectLocalizedString(K) ConnectL10N::GetString(ConnectL10N::K())
