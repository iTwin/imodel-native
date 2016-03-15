/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/HttpError.xliff.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <BeSQLite/L10N.h>

//=======================================================================================
// @bsiclass
//=======================================================================================
BENTLEY_TRANSLATABLE_STRINGS_START(HttpErrorL10N, HttpError)
    L10N_STRING(NL10N_Invalid)                              // =="Invalid Localization Id"==
    L10N_STRING(MSG_ConnectionStatus_CouldNotConnect)       // =="Could not connect to the server. Please check your internet connection and try again"==
    L10N_STRING(MSG_ConnectionStatus_Timeout)               // =="Connection timeout. Please check your internet connection and try again"==
    L10N_STRING(MSG_ConnectionStatus_ConnectionLost)        // =="Connection lost. Please check your internet connection and try again"==
    L10N_STRING(MSG_ConnectionStatus_CertificateError)      // =="Server connection is untrusted. Contact your server administrator"==
    L10N_STRING(MSG_ConnectionStatus_UnknownStatus)         // =="Unknown connection error"==
    L10N_STRING(MSG_HttpErrorDescription)                   // =="Http error: %d"==
    L10N_STRING(STATUS_HttpStatus_401)                      // =="Could not login with specified credentials"==
    L10N_STRING(STATUS_HttpStatus_403)                      // =="Permission denied"==
    L10N_STRING(STATUS_HttpStatus_404)                      // =="Resource not found"==
    L10N_STRING(STATUS_HttpStatus_429)                      // =="Too many requests. Please try again later"==
    L10N_STRING(STATUS_ServerError)                         // =="Server error happened. Please contact your administrator"==
    L10N_STRING(STATUS_UnexpectedStatus)                    // =="Unexpected error happened. Please contact your administrator"==
BENTLEY_TRANSLATABLE_STRINGS_END

#define HttpErrorLocalizedString(K) HttpErrorL10N::GetString(HttpErrorL10N::K())
